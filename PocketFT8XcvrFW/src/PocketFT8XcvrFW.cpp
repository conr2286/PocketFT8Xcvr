
/*
** NAME
**  PocketFT8XcvrFW -- Firmware for the Pocket FT8 Revisited Software Defined Radio Transceiver
**
** DESCRIPTION
**  The KQ7B PocketFT8Xcvr is a derivative of the original Pocket FT8 transceiver
**  in a compact, self-contained, package.  Notable features include:
**    + 4-Layer PCB
**    + Single band FT8 transmit/receiver
**    + Teensy 4.1 processor
**    + 4.0x2.8" PCB mates with the Adafruit display
**    + Adafruit 320x480 3.5" resistive touchscreen display
**    + TCXO
**    + Powered by 0.37 Amps from a single +5V USB power source
**    + SD Card logging to ADIF file
**    + Station configuration JSON file
**    + Plugable filters
**    + GPS disciplined UTC time and location
**    + Optional Adafruit Ultimate GPS automates time synchronization and Maidenhead Grid Square locator
**  The overall goal is to deliver a single band, fully self-contained, HF FT8 transceiver for
**  POTA/SOTA work where light weight and power requirements dominate other goals.
**
** RATIONAL
**  + Single-band operation is not a serious limitation for SOTA/POTA work where the portable
**  antenna limits the available band(s).
**  + QRPP == battery life for portable work
**  + While the SI4735 receiver's performance likely lags that available from a Tayloe detector,
**  it offers considerable simplicity and effectiveness for SOTA/POTA FT8 operation and will
**  likely stations that here the QRPP signal
**
** FUTURE
**    + Consider using a real touchscreen controller to replace the MCP342X ADC
**    + Bug fixes (see https://github.com/conr2286/PocketFT8Xcvr Issues)
**    + Update the FT8 library with Karlis' more recent code
**
** KQ7B (conr2286 AT gmail.com)
**  Retired engineer living in north central Idaho (USA), stereotype "old guy"
**  ham radio operator, began constructing radios and first licensed in the 1960s.
**  "What a long, strange trip it has been." -- The Grateful Dead
**
** VERSIONS
**  1.00 Never completed due to unobtanium componentry
**  1.01 PCB required several patches to correct known issues.  You don't want this.
**  1.10 Working firmware and transmitter but SI4735 suffers serious I2C noise issues.  Avoid this one too.
**  2.00 Revised board/firmware overcomes I2C noise problems.  Requires one patch wire for GPS.
**  2.10 Firmware implements bug fixes, ADIF logging, and the "RoboOp" QSO Sequencer.  Uses V2.00 boards.
**
** ATTRIBUTION
**  https://github.com/Rotron/Pocket-FT8  Charley Hill's original Pocket FT8 project
**  https://github.com/WB2CBA/W5BAA-FT8-POCKET-TERMINAL  Barb's FT8 Pocket Terminal
**  https://github.com/kgoba/ft8_lib  Karlis Goba's FT8 Library for microprocessors
**  https://github.com/conr2286/PocketFT8Xcvr  The KQ7B HW and FW projects
**  Many other contributors (e.g. Adafruit et al) to the various libraries
**
** LICENSES
**  + The permissive MIT license widely cited throughout the Pocket FT8 firmware and libraries
**  + Excepting the SI5351 library has a copy-left GPL license.
**
** REFERENCES
**  Franke, Somerville, and Taylor.  "The FT4 and FT8 Communication Protocols." QEX. Jul/Aug 2020.
*/
#include <Adafruit_GFX.h>  //WARNING:  *Must* #include ADAfruit_GFX.h prior to HX8357_t3n.h
#include <Arduino.h>
#include <Audio.h>
#include <AudioStream.h>
#include <EEPROM.h>
#include <SD.h>
#include <SI4735.h>
#include <SPI.h>
#include <TimeLib.h>
#include <gfxfont.h>

#include "AListBox.h"
#include "Config.h"
#include "FT8Font.h"
#include "GPShelper.h"
#include "HX8357_t3n.h"  //WARNING:  Adafruit_GFX.h must include prior to HX8357_t3n.h
#include "LogFactory.h"
#include "MCP342x.h"
#include "DEBUG.h"
#include "PocketFT8Xcvr.h"
#include "Process_DSP.h"
#include "Sequencer.h"
#include "Timer.h"
#include "TouchScreen_I2C.h"
#include "UserInterface.h"
#include "WF_Table.h"
#include "arm_math.h"
#include "button.h"
#include "constants.h"
#include "decode_ft8.h"
#include "gen_ft8.h"
#include "locator.h"
#include "maidenhead.h"
#include "patch_full.h"  // SSB patch for whole SSBRX full download
#include "pins.h"
#include "si5351.h"
#include "traffic_manager.h"

// Forward references (required to build on PlatformIO)
void loadSSB();
time_t getTeensy3Time();
void waitForFT8timeslot();
void process_data();
void update_synchronization();
static void copy_to_fft_buffer(void *, const void *);

// Enable comments in the JSON configuration file
#define ARDUINOJSON_ENABLE_COMMENTS 1
#include <ArduinoJson.h>

// We include .../teensy4/AudioStream.h only to confirm AUDIO_SAMPLE_RATE_EXACT is 6400.0f
#include <AudioStream.h>

// Si4735
#define AM_FUNCTION 1
#define USB 2

// Build the receiver
#define MINIMUM_FREQUENCY 7000  // Min freq supported by HW filters
#define MAXIMUM_FREQUENCY 7300  // Max freq supported by HW filters
SI4735 si4735;                  // The receiver

// Define the static objects widely referenced throughout PocketFT8Xcvr
Station thisStation(MINIMUM_FREQUENCY, MAXIMUM_FREQUENCY);  // Station model
ConfigType config;                                          // RAM-resident copy of CONFIG.JSON parameters
UserInterface ui;                                           // User Interface
Si5351 si5351;                                              // Transmitter/receiver's clock

// Teensy Audio Library setup (don't forget to install AudioStream6400.h in the Arduino teensy4 library folder)
AudioInputAnalog adc1;  // xy=132,104
AudioRecordQueue queue1;
AudioConnection patchCord2(adc1, queue1);
static const unsigned audioQueueSize = 100;  // Number of blocks in the Teensy audio queue (one symbol's period requires 8 blocks)

// Audio pipeline buffers
q15_t dsp_buffer[3 * input_gulp_size] __attribute__((aligned(4)));
q15_t dsp_output[FFT_SIZE * 2] __attribute__((aligned(4)));  // TODO:  Move to DMAMEM?
q15_t input_gulp[input_gulp_size] __attribute__((aligned(4)));

// Global flag to disable the transmitter for testing
// bool disable_xmit = false;  // Flag can be set with config params

// Legacy globals
long et1 = 0, et2 = 0;
const uint16_t size_content = sizeof ssb_patch_content;  // see ssb_patch_content in patch_full.h or patch_init.h;
uint32_t current_time, start_time, ft8_time;
uint32_t days_fraction, hours_fraction, minute_fraction;
uint8_t ft8_hours, ft8_minutes, ft8_seconds;
int FT_8_counter, ft8_marker;

// Apparently set when it's time to do an FFT (When is that?  At the end of a received symbol?????)
int DSP_Flag;

// Apparently set when the timeslot's received messages are ready to be decoded
int decode_flag;

// Initialized to 1 by setup() and the multitude of synchronization functions.  Set to 0 by
// process_FT8_FFT() apparently at the end of a receive timeslot???
int ft8_flag;

int WF_counter;
int num_decoded_msg;

// Apparently set when the transmitted carrier is on
int xmit_flag;

// Set when a transmission is pending the beginning of the next timeslot
int Transmit_Armned;

// This appears to be the modulated symbol counter for payload and costas
int ft8_xmit_counter;
int master_decoded;

int tune_flag;

int log_flag, logging_on;  // TODO:  deprecate

// Get a reference to the QSO Sequencer machine implementing a robo-like operator handling
// QSOs arising from our own CQ and calls to a remote station
Sequencer &seq = Sequencer::getSequencer();

// Build the GPSHelper encapsulating the details of operating the GPS receiver
GPShelper gpsHelper(9600);

/**
 * @brief Callback function invoked by GPShelper while it acquires a GPS fix
 *
 * @param seconds Number of seconds elapsed while acquiring the GPS fix
 *
 * GPShelper calls here ~once/second
 *
 * Note:  The display must be initialized before GPShelper calls here
 *
 * TODO:  Deprecated
 *
 **/
static void gpsCallback(unsigned seconds) {
    // char msg[32];
    // snprintf(msg, sizeof(msg), "Acquiring GPS fix:  %03d", seconds);
    // ui.applicationMsgs->setText(msg);
}  // gpsCallback()

/**
 ** @brief Sketch initialization
 **
 ** The FLASHMEM qualifier places the setup() function in the Teensy 4.1 flash memory, thereby
 ** saving RAM1 for high performance code/data.
 **/
FLASHMEM void setup(void) {
    // Get the USB serial port running before something else goes wrong
    Serial.begin(9600);
    DTRACE();

    // Is Teensy recovering from a crash?
    if (CrashReport) {
        Serial.print(CrashReport);  // You'll have fun debugging this on a Teensy :(
        delay(5000);
    }

    // Get the UI running
    ui.begin();
    ui.applicationMsgs->setText("Starting");

    // Confirm firmware built with the modified teensy4/AudioStream.h library file in the Arduino IDE.  Our FT8 decoder
    // won't run at the standard Teensy sample rate.  In the best-of-all-possible-worlds, we'd implement this check at
    // compile time, but KQ7B hasn't found how to check at compile-time with a float value for AUDIO_SAMPLE_RATE_EXACT.
    if (AUDIO_SAMPLE_RATE_EXACT != 6400.0f) {
        ui.applicationMsgs->setText("FATAL:  AUDIO_SAMPLE_RATE_EXACT!=6400.0F", A_RED);
        Serial.println("FATAL:  You *must* copy AudioStream6400.h to .../teensy/hardware/avr/1.59.0/cores/teensy4/AudioStream.h\n");
        Serial.println("...before building the Pocket FT8 Revisited firmware.\n");
        while (true) continue;  // Fatal
    }

    // Turn the transmitter off and the receiver on
    thisStation.setEnableTransmit(false);  // Disable transmitter until we get config info
    pinMode(PIN_PTT, OUTPUT);
    pinMode(PIN_RCV, OUTPUT);
    digitalWrite(PIN_RCV, HIGH);  // Disable the PA and disconnect receiver's RF input from antenna
    digitalWrite(PIN_PTT, LOW);   // Unground the receiver's RF input

    // Initialize the SD library if the card is available
    if (!SD.begin(BUILTIN_SDCARD)) {
        ui.applicationMsgs->setText("ERROR:  Unable to access SD card");
        delay(2000);
    }

    // Initialize the SI5351 clock generator.  NOTE:  PocketFT8Xcvr boards use CLKIN input (supposedly less jitter than XTAL).
    si5351.init(SI5351_CRYSTAL_LOAD_8PF, 25000000, 0);          // KQ7B's counter isn't accurate enough to calculate a correction
    si5351.set_pll_input(SI5351_PLLA, SI5351_PLL_INPUT_CLKIN);  // We are using cmos CLKIN, not a XTAL input!!!
    si5351.set_pll_input(SI5351_PLLB, SI5351_PLL_INPUT_CLKIN);  // All PLLs using CLKIN
    si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);              // Fixed point division offers less jitter
    si5351.set_freq(3276800, SI5351_CLK2);                      // Receiver's fixed frequency clock for Si4735
    si5351.output_enable(SI5351_CLK2, 1);                       // Receiver's clock is always on
    si5351.output_enable(SI5351_CLK0, 0);                       // Disable transmitter for now

    // Gets and sets the Si47XX I2C bus address
    int16_t si4735Addr = si4735.getDeviceI2CAddress(PIN_RESET);
    if (si4735Addr == 0) {
        ui.applicationMsgs->setText("FATAL:  Si473x not found");
        while (1) continue;  // Fatal
    } else {
        // DPRINTF("The Si473X I2C address is 0x%2x\n", si4735Addr);
    }

    // Digest the CONFIG.JSON file
    readConfigFile();
    thisStation.setCallsign(config.callsign);      // Extract callsign from CONFIG.JSON
    thisStation.setLocator(config.locator);        // Extract optional locator from CONFIG.JSON
    thisStation.setFrequency(config.frequency);    // Extract frequency from CONFIG.JSON
    thisStation.setMyName(config.myName);          // Operator's personal name (not callsign)
    thisStation.setQSOtimeout(config.qsoTimeout);  // Seconds RoboOp will retransmit without receiving a response
    thisStation.setSOTAref(config.my_sota_ref);    // This station's SOTA Reference if any

    // Initialize the SI4735 receiver
    delay(10);
    et1 = millis();
    loadSSB();
    et2 = millis();
    delay(10);
    si4735.setTuneFrequencyAntennaCapacitor(1);  // Set antenna tuning capacitor for SW.
    delay(10);
    si4735.setSSB(MINIMUM_FREQUENCY, MAXIMUM_FREQUENCY, config.frequency, 1, USB);  // FT8 is *always* USB
    delay(10);
    // currentFrequency = si4735.getFrequency();
    si4735.setVolume(50);

    // Initialize the receiver's DSP chain
    init_DSP();
    initalize_constants();

    // Setup buffers for received audio.  Extensive testing discovered smaller buffer pool sizes
    //(e.g. 20) were sometimes exhausted, especially following certain HX8357 graphics activity.
    // It can run on less than 100, but you'll occasionally miss a receive timeslot.
    AudioMemory(audioQueueSize);  // Number of Teensy audio queue blocks

    // Start the audio pipeline
    queue1.begin();

    // Set operating frequency
    set_startup_freq();
    delay(10);

    ui.displayFrequency();

    // Sync MCU clock with battery-backed RTC
    setSyncProvider(getTeensy3Time);
    ui.displayDate();  // Likely not yet GPS disciplined
    ui.displayTime();  //...and thus displayed in YELLOW

    // Final station initialization
    thisStation.setRig(String("https://github.com/conr2286/PocketFT8Xcvr"));
    set_Station_Coordinates(thisStation.getLocator());              // Configure the Maidenhead Locator library with grid square
    ui.displayLocator(String(thisStation.getLocator()), A_YELLOW);  // Display the locator with caution yellow until we get GPS fix
    ui.displayCallsign();                                           // Display station callsigne

    // Notify operator if transmitter is disabled
    if (!thisStation.canTransmit()) ui.applicationMsgs->setText("Transmitter disabled");

    // Start the QSO Sequencer (RoboOp) and receiver
    seq.begin(thisStation.getQSOtimeout(), config.logFilename);  // Parameter configures Sequencer's run-on QSO timeout and the logfile name
    receive_sequence();                                          // Setup to receive at start of first timeslot

    // Wait for the first FT8 timeslot (at 0, 15, 30, or 45 seconds past the minute) to begin
    start_time = millis();  // Note start time for update_synchronization()
    waitForFT8timeslot();   // Wait for a 15 second FT8 timeslot

}  // setup()

unsigned oldFlags = 0;  // Used only for debugging the flags

/**
 * @brief Here's the Arduino world's main loop()
 *
 * Note:  Placing the loop() code in FLASHMEM saves RAM1 memory for more time-sensitive activities
 */
FLASHMEM void loop() {
    // Check station params to determine if we have everything required to transmit
    if (thisStation.canTransmit()) thisStation.setEnableTransmit(true);

    // If it's not time to decode incoming messages, then it's time to grab the recv'd sigs from A/D
    if (decode_flag == 0) process_data();

    // Is there buffered time-domain data for the DSP logic to work with???
    if (DSP_Flag == 1) {
        // Yes, transform recv'd time-domain audio to frequency domain (to see the FT8 sigs)
        process_FT8_FFT();

        // Is a transmission in-progress?  Why is this question guarded by DSP_Flag (above)???????
        if (xmit_flag == 1) {
            int offset_index = 5;  //???

            // Is it time to modulate the SI5351 with the transmitter's next FSK tone?
            if (ft8_xmit_counter >= offset_index && ft8_xmit_counter < 79 + offset_index) {
                // Each tone transmits 3 bits of message
                set_FT8_Tone(tones[ft8_xmit_counter - offset_index]);  // Program new FT8 tone into the SI5351
            }

            ft8_xmit_counter++;  // Apparently counting tones in the FSK transmission

            // Is it time to switch from transmitting to receiving (i.e. all tones sent) at the end of our timeslot?
            if (ft8_xmit_counter == 80 + offset_index) {
                // DPRINTF("End of transmit timeslot\n");
                xmit_flag = 0;               // Indication that the transmission has ended
                receive_sequence();          // Switch HW from transmitting to receiving
                terminate_transmit_armed();  // Switch again then update GUI
            }
        }  // xmit_flag

        DSP_Flag = 0;
        ui.displayDate();
        ui.displayTime();

    }  // DSP_Flag

    // Apparently:  Have we acquired all of the timeslot's receiver time-domain data?
    if (decode_flag == 1) {
        // unsigned long td0 = millis();
        num_decoded_msg = ft8_decode();  // Decode the received messages
        master_decoded = num_decoded_msg;
        decode_flag = 0;

        // If a message is waiting for transmission, turn-on the carrier and set xmit_flag to modulate it.
        // WARNING:  There may be some confusion about what Transmit_Armned really means.  But this is
        // legacy code and we're hesitant to modify it while the ghosts-of-versions-past still haunt us.
        if (Transmit_Armned == 1) setup_to_transmit_on_next_DSP_Flag();
    }

    // Check touchscreen and serial port for activity
    pollTouchscreen();
    // if (tune_flag == 1) process_serial();  // TODO:  Do we still need this???

    // If we haven't recorded valid GPS data, but the GPS device has now acquired a fix, then obtain the GPS data.
    // This is a bit abrupt as we afterward more or less resynch everything and wait for a timeslot.
    if (!gpsHelper.validGPSdata && gpsHelper.hasFix()) {
        // DPRINTF("gpsHelper.validGPSdata=%d, gpsHelper.hasFix()=%d\n", gpsHelper.validGPSdata, gpsHelper.hasFix());

        // Sync MCU and RTC time with GPS if it has valid data
        if (gpsHelper.obtainGPSData(config.gpsTimeout, gpsCallback)) {
            // Inform operator
            ui.applicationMsgs->setText("GPS has acquired a fix");

            // Set the MCU time to the GPS result
            setTime(gpsHelper.hour, gpsHelper.minute, gpsHelper.second, gpsHelper.day, gpsHelper.month, gpsHelper.year);

            // Now set the battery-backed Teensy RTC to the GPS-derived time in the MCU
            Teensy3Clock.set(now());

            // Use the GPS-derived locator unless config.json hardwired it to something else
            if (strlen(config.locator) == 0) {
                thisStation.setLocator(get_mh(gpsHelper.flat, gpsHelper.flng, 4));
                // strlcpy(Locator, get_mh(gpsHelper.flat, gpsHelper.flng, 4), sizeof(Locator));
                ui.displayLocator(thisStation.getLocator(), A_GREEN);
            }

            // Arrange for the Teensy battery-backed RTC (UTC) to keep the MCU time accurate
            setSyncProvider(getTeensy3Time);

            // Record the locator gridsquare for logging
            set_Station_Coordinates(thisStation.getLocator());

            // Update date/time in the UI
            ui.displayDate(true);
            ui.displayTime();

            // Wait for an FT8 timeslot to begin (this updates start_time) using GPS disciplined time
            waitForFT8timeslot();
        }
    }  // gps

    // Service the Timer inventory
    Timer::serviceTimers();

    // Update flags when a new timeslot begins.
    update_synchronization();

}  // loop()

time_t getTeensy3Time() {
    return Teensy3Clock.get();
}

/**
 * @brief Loads the SSB patch into the SI4735
 */
FLASHMEM void loadSSB() {
    si4735.queryLibraryId();  // Is it really necessary here? I will check it.
    si4735.patchPowerUp();
    // si4735.setPowerUp(1, 1, 1, 0, 1, SI473X_ANALOG_DIGITAL_AUDIO);
    // si4735.radioPowerUp();
    delay(50);
    si4735.downloadPatch(ssb_patch_content, size_content);
    // Parameters
    // AUDIOBW - SSB Audio bandwidth; 0 = 1.2KHz (default); 1=2.2KHz; 2=3KHz; 3=4KHz; 4=500Hz; 5=1KHz;
    // SBCUTFLT SSB - side band cutoff filter for band passand low pass filter ( 0 or 1)
    // AVC_DIVIDER  - set 0 for SSB mode; set 3 for SYNC mode.
    // AVCEN - SSB Automatic Volume Control (AVC) enable; 0=disable; 1=enable (default).
    // SMUTESEL - SSB Soft-mute Based on RSSI or SNR (0 or 1).
    // DSP_AFCDIS - DSP AFC Disable or enable; 0=SYNC MODE, AFC enable; 1=SSB MODE, AFC disable.
    // si4735.setSSBConfig(bandwidthIdx, 1, 0, 1, 0, 1);
    si4735.setSSBConfig(2, 1, 0, config.enableAVC, 0, 1);  // 2 = 3 kc bandwidth
    // DPRINTF("SI4735 AVC = %u\n", config.enableAVC);
}

/**
 * Apparently copies queued blocks of received audio into the FFT buffer
 *
 * Does nothing if there's not enough (8 blocks) audio data available.
 *
 * Does ??? with dsp_buffer[]
 *
 * Sets DSP_Flag when there's work for the DSP logic
 *
 * There's likely a dependency that block_size==AUDIO_BLOCK_SIZE???
 *
 * Note:  If we sample for 12.64 seconds at 6400 samples/second, then we
 * anticipate acquiring 80896 samples during a complete timeslot.
 *
 * Note:  num_que_blocks (8) is the number of blocks required to capture
 * one symbol (0.160 seconds) of received audio.
 *
 **/
void process_data() {
    unsigned count = queue1.available();
    if (count >= audioQueueSize) DPRINTF("*** Audio queue filled with %u blocks ***\n", count);
    if (count >= num_que_blocks) {
        // Copy received audio from queue buffers to the FFT buffer
        for (int i = 0; i < num_que_blocks; i++) {
            copy_to_fft_buffer(input_gulp + block_size * i, queue1.readBuffer());
            queue1.freeBuffer();
        }

        for (int i = 0; i < input_gulp_size; i++) {
            dsp_buffer[i] = dsp_buffer[i + input_gulp_size];
            dsp_buffer[i + input_gulp_size] = dsp_buffer[i + 2 * input_gulp_size];
            dsp_buffer[i + 2 * input_gulp_size] = input_gulp[i];
        }

        // There is apparently work to do for the DSP logic
        DSP_Flag = 1;
    }
}  // process_data()

/**
 * Copies one audio block of 16-bit words from source to destination
 *
 * @param destination Destination buffer
 * @param source Source buffer
 *
 * The AUDIO_BLOCK_SIZE appears to be cast-in-brass at 128 in AudioStream6400.h
 *
 **/
static void copy_to_fft_buffer(void *destination, const void *source) {
    const uint16_t *src = (const uint16_t *)source;
    uint16_t *dst = (uint16_t *)destination;
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        *dst++ = *src++;  // real sample plus a zero for imaginary
    }

    // //Configurable recording of raw 16-bit audio at 6400 samples/second to an SD file
    // if (ft8Raw != NULL) {
    //   ft8Raw.write(source, AUDIO_BLOCK_SAMPLES * sizeof(uint16_t));
    //   recordSampleCount += AUDIO_BLOCK_SAMPLES;  //Increment count of recorded samples
    //   if (recordSampleCount % (unsigned)AUDIO_SAMPLE_RATE == 0) {
    //     DPRINTF("Audio recording in progress...\n");  //One second progress indicator
    //   }
    // }

}  // copy_to_fft_buffer()

// /**
//  * Updates synchronization using Teensy's TimeLib
//  *
//  * Accuracy limited to +/- 1 second
//  *
//  * Appears to be dead code
//  **/
// void rtc_synchronization() {
//   DPRINTF("*****rtc_synchronization()\n");
//   getTeensy3Time();

//   if (ft8_flag == 0 && second() % 15 == 0) {
//     ft8_flag = 1;
//     FT_8_counter = 0;
//     ft8_marker = 1;
//     WF_counter = 0;
//   }
// }

/**
 * Update timeslot synchronization using Arduino's elapsed time, millis()
 *
 * Note the objective here is to poll the current time and setup the various
 * flags when the next timeslot begins, not to wait (block) for that timeslot
 * to begin.  The flags cause loop() to perform whatever receive/transmit activities
 * are required in that timeslot.
 *
 * Because we are using millis() with millisecond accuracy, this approach to find the
 * beginning of a timeslot is likely accurate, assuming start_time represents
 * the number of elapsed milliseconds recorded at the *true* (UTC) start of a timeslot
 * (i.e. the 0, 15, 30 or 45 second boundary).  Note that waitForFT8timeslot()
 * initializes start_time at the beginning of the first timeslot; if that time is
 * accurate (see discussion in waitForFT8timeslot), then update_synchronization() may
 * accurately determine the beginning of subsequent timeslots.
 *
 **/
static unsigned long nextTimeSlot;
void update_synchronization() {
    current_time = millis();
    ft8_time = current_time - start_time;  // mS elapsed in current interval???

    // ft8_hours = (int8_t)(ft8_time / 3600000);
    // hours_fraction = ft8_time % 3600000;
    // ft8_minutes = (int8_t)(hours_fraction / 60000);
    // ft8_seconds = (int8_t)((hours_fraction % 60000) / 1000);

    // Charlie's original sync decision used 200 mS now 160 mS (one FT8 symbol time)
    // TODO:  Is our time accurate enough to reduce window below 160 mS???
    if (ft8_flag == 0 && ft8_time % 15000 <= 160) {
        ft8_flag = 1;
        FT_8_counter = 0;
        ft8_marker = 1;
        WF_counter = 0;

        // Notify sequencer
        seq.timeslotEvent();  // Increments sequence number for upcoming timeslot

        // Debug missed timeslots (we are too late to receive the first symbol)
        if (current_time > nextTimeSlot + 160) {
            DPRINTF("*** Missed timeslot:  ft8_time modulo 15000=%lu, current_time=%lu, nextTimeSlot=%lu, autoReplyToCQ=%u *****************************\n", ft8_time % 15000, current_time, nextTimeSlot, getAutoReplyToCQ());
        }
        nextTimeSlot = current_time + 15000;

        // Update the displayed date
        ui.displayDate(true);  // Force an update so display will change from yellow to green if GPS is acquired

        // Debug timeslot and sequencer problems
        DPRINTF("-----Timeslot %lu:  Sequencer.state=%u, Transmit_Armned=%u, xmit_flag=%u, message='%s', autoReplyToCQ=%u -------------------\n", seq.getSequenceNumber(), seq.getState(), Transmit_Armned, xmit_flag, get_message(), getAutoReplyToCQ());
    }
}  // update_synchronization()

/**
 * This function appears to manually sync to an FT8 timeslot
 *
 * FT8 timeslots begin on 15 second boundaries (e.g. 0, 15, 30 or 45 seconds into a minute).
 * This function appears to be invoked only by the Sy button press to manually sync to (i.e. begin)
 * a timeslot when the station operator has identified (by observing an accurate clock?  by listening
 * to FT8 traffic on a different receiver?) the beginning of a timeslot.
 *
 * If we had a dependable satellite receiver (or other super accurate clock), would the Sy button
 * and this function still be necessary???
 *
 **/
void sync_FT8(void) {
    // DPRINTF("*****sync_FT8()\n");

    setSyncProvider(getTeensy3Time);  // commented out?

    start_time = millis();
    ft8_flag = 1;
    FT_8_counter = 0;
    ft8_marker = 1;
    WF_counter = 0;
}

/**
 *  Wait for a 15-second FT8 timeslot (e.g. 0, 15, 30 or 45 seconds into a minute)
 *
 *  @return At the beginning of the next upcoming FT8 timeslot
 *
 *  Unlike update_syncronization(), this function blocks (waits) until the next
 *  timeslot begins.  Then it initializes the various flags so loop() can perform
 *  whatever activities (e.g. transmit or receive) are needed in that timeslot.
 *  When waitForFT8timeslot() returns, the next timeslot has begun.
 *
 *  The GPS, when available, is vastly more accurate than the Teensy RTC as it's
 *  sync'd to the satellites and has millisecond, rather than second, resolution.
 *  But when GPS is unavailable, we use the RTC and hope the costas symbols will
 *  enable us to decode received messages.
 *
 *  Since we don't wish to frequently poll the GPS for the current UTC, we
 *  have gpsHelper sync the Arduino's millis() to the GPS by recording the
 *  value of millis() at moment when the GPS acquired the UTC time.  Then,
 *  as we prepare to await a timeslot, we calculate how many milliseconds
 *  to wait.
 *
 **/
void waitForFT8timeslot(void) {
    // DPRINTF("waitForFT8timeslot() gpsHelper.validFix=%u\n", gpsHelper.validGPSdata);

    // displayInfoMsg("Waiting for timeslot");
    ui.applicationMsgs->setText("Awaiting FT8 timeslot");

    // If we have valid GPS data, then use GPS time for milliseconds rather than second resolution
    if (gpsHelper.validGPSdata) {
        // Calculate the value of elapsed millis() when the next FT8 timeslot will begin
        unsigned long msGPS = gpsHelper.second * 1000 + gpsHelper.milliseconds;  // Milliseconds into the minute when setup acquired UTC date/time from GPS
        unsigned long msGPS2FT8 = 15000 - msGPS % 15000;                         // Milliseconds between GPS acquisition and start of next FT8 timeslot
        unsigned long millisAtFT8 = gpsHelper.elapsedMillis + msGPS2FT8;         // Elapsed runtime millis() at start of next FT8 timeslot (might be in the past)
        while (millis() < millisAtFT8) continue;                                 // Wait for elapsed runtime to reach/exceed start of next FT8 timeslot
        DPRINTF("msGPS=%lu, msGPS2FT8=%lu, millis()=%lu\n", msGPS, msGPS2FT8, millis());

        // When we don't have valid GPS data, then we fall back to using Teensy timelib's 1 second clock resolution:(
    } else {
        // Wait for the end of the current 15 second FT8 timeslot.  This will arise at 0, 15, 30 or 45 seconds past the current minute.
        // Sadly... second() has only 1000 ms resolution and, without GPS, its accuracy may not be what FT8 needs.
        while ((second()) % 15 != 0) continue;
        // DTRACE();
    }

    // Begin the first FT8 timeslot
    // DTRACE();
    start_time = millis();              // Start of the first timeslot in ms of elapsed execution
    nextTimeSlot = start_time + 15000;  // Time (ms) when next timeslot should begin
    ft8_flag = 1;
    FT_8_counter = 0;
    ft8_marker = 1;
    WF_counter = 0;

    // Notify the sequencer
    seq.timeslotEvent();

    // Update display
    // displayInfoMsg("RECV");
    // displayInfoMsg(" ");
    ui.applicationMsgs->setText("Ready");

    DPRINTF("-----Timeslot %lu: second()=%u, millis()=%lu ---------------------\n", seq.getSequenceNumber(), millis());

}  // waitForFT8timeslot()
