/*
** NAME
**  PocketFT8XcvrFW -- Firmware for the KQ7B edition of the Pocket FT8 transceiver
**
** DESCRIPTION
**  This is a mostly faithful implementation of the original Pocket FT8 transceiver
**  in a compact, self-contained, package.  Notable features include:
**    + 4-Layer PCB
**    + FT8 transmit/receiver
**    + Teensy 4.1 processor
**    + Small, 4.0x2.8" PCB mates with the Adafruit display
**    + Adafruit 320x480 3.5" resistive touchscreen display
**    + TCXO
**    + Powered by 0.37 Amps from a single +5V USB power source 
**    + SD Card logging to txt file
**    + Station configuration JSON file
**  The overall goal is to construct a single band, self-contained, HF FT8 transceiver for
**  POTA/SOTA work where light weight and power requirements dominate other requirements.
**
** FUTURE
**    + Support for GPS-sourced grid square
**    + Standard log file format
**    + Wouldn't it be nice if the SD card were accessible via USB from a computer host
**    + Bug fixes (see https://github.com/conr2286/PocketFT8Xcvr Issues)
**
** KQ7B
**  Retired engineer living in north central Idaho (USA)
**
**  VERSIONS
**  1.00 Never built due to unobtanium componentry
**  1.01 PCB requires several patches to correct known issues
**  1.10 Working firmware and transmitter but SI4735 suffers from I2C noise issues
**  2.00 (In Progress) Revised board/firmware to overcome I2C noise problem
**  
** ATTRIBUTION
**  https://github.com/Rotron/Pocket-FT8  Charley Hill's original Pocket FT8
**  https://github.com/WB2CBA/W5BAA-FT8-POCKET-TERMINAL  Barb's FT8 Pocket Terminal
**  https://github.com/kgoba/ft8_lib  Karlis Goba's FT8 Library for microprocessors
**  https://github.com/conr2286/PocketFT8Xcvr  The KQ7B HW and FW edition
**  Many others' contributions to the various libraries
**
** LICENSES
**  Various open source licenses widely cited throughout
*/

#include "DEBUG.h"
#include <Audio.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include "HX8357_t3n.h"
#include "TouchScreen_I2C.h"
#include "MCP342x.h"
#include "si5351.h"
#include <SI4735.h>
#include "patch_full.h"  // SSB patch for whole SSBRX full download
#include <TimeLib.h>
#include <EEPROM.h>
#include <TinyGPS.h>


#include "Process_DSP.h"
#include "decode_ft8.h"
#include "WF_Table.h"
#include "arm_math.h"
#include "display.h"
#include "button.h"
#include "locator.h"
#include "traffic_manager.h"
#include "pins.h"

#include "Arduino.h"
#include "AudioStream.h"
#include "arm_math.h"
#include "constants.h"

//GPS Stuff
TinyGPS gps;
#define SerialGPS Serial1   //Teensy 4.1 Serial Port 1, pins 0 and 1
#define GPS_BAUD_RATE 9600  //Adafruit Ultimate GPS baud rate
float flat, flon;           //GPS-derived lattitude and longitude

//Enable comments in the JSON configuration file
#define ARDUINOJSON_ENABLE_COMMENTS 1
#include <ArduinoJson.h>

#define AM_FUNCTION 1
#define USB 2

//For HW debugging, define an option to record received audio to an SD file, ft8.raw,
//encoded as a single channel of 16-bit unsigned integers at 6400 samples/second.
//The command, sox -r 6400 -c 1 -e unsigned -b 16 ft8.raw ft8.wav, will convert the
//recording to a wav file.  Set the duration to 0 to eliminate file I/O overhead.
#define AUDIO_RECORDING_FILENAME "ft8.raw"
//#define AUDIO_SAMPLE_RATE 6400

//Configuration parameters read from SD file
#define CONFIG_FILENAME "/config.json"
struct Config {
  char callsign[12];                     //11 chars and NUL
  char location[5];                      //4 char maidenhead locator and NUL
  unsigned frequency;                    //Operating frequency in kHz
  unsigned long audioRecordingDuration;  //Seconds or 0 to disable audio recording
  unsigned enableAVC;                    //0=disable, 1=enable SI47xx AVC
} config;

//Default configuration
#define DEFAULT_FREQUENCY 7074                //kHz
#define DEFAULT_CALLSIGN "NOCALL"             //There's no realistic default callsign
#define DEFAULT_LOCATION "****"               //Will later obtain the default maidenhead square from GPS if we get a lock
#define DEFAULT_AUDIO_RECORDING_DURATION 0UL  //Default of 0 seconds disables audio recording
#define DEFAULT_ENABLE_AVC 1                  //SI4735 AVC enabled by default

//Define lower/upper frequency limitations of the 40m hardware implementation
#define MINIMUM_FREQUENCY 7000  //Low edge of band in kHz
#define MAXIMUM_FREQUENCY 7300  //Upper edge of band in kHz

//Adafruit 480x320 touchscreen configuration
HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_RST, PIN_MOSI, PIN_DCLK, PIN_MISO);  //Teensy 4.1 pins
TouchScreen ts = TouchScreen(PIN_XP, PIN_YP, PIN_XM, PIN_YM, 282);                   //The 282 ohms is the measured x-Axis resistance of 3.5" Adafruit touchscreen in 2024

///Build the VFO and receiver objects
Si5351 si5351;
SI4735 si4735;

//Teensy Audio Library setup (don't forget to install AudioStream32k.h! in the library folder)
AudioInputAnalog adc1;  //xy=132,104
AudioRecordQueue queue1;
AudioConnection patchCord2(adc1, queue1);

//Optional (see config file) raw audio recording file written to SD card
File ft8Raw = NULL;
unsigned long recordSampleCount = 0;  //Number of 16-bit audio samples recorded so far

//Audio pipeline buffers
q15_t dsp_buffer[3 * input_gulp_size] __attribute__((aligned(4)));
q15_t dsp_output[FFT_SIZE * 2] __attribute__((aligned(4)));
q15_t input_gulp[input_gulp_size] __attribute__((aligned(4)));

//ToDo:  Arrange for the various modules to access these directly from config structure
char Station_Call[12];  //six character call sign + /0
char Locator[5];        // four character locator  + /0

uint16_t currentFrequency;
long et1 = 0, et2 = 0;
const uint16_t size_content = sizeof ssb_patch_content;  // see ssb_patch_content in patch_full.h or patch_init.h;
uint32_t current_time, start_time, ft8_time;
uint32_t days_fraction, hours_fraction, minute_fraction;

uint8_t ft8_hours, ft8_minutes, ft8_seconds;

int FT_8_counter, ft8_marker;

//Apparently set when there's received audio in the FFT buffers
int DSP_Flag;

//Apparently set when the received messages are ready to be decoded
int decode_flag;

//Apparently set when it's time for the FFT to compute the magnitudes of a received signal
int ft8_flag;

//Apparently set when CQ button pressed
extern int CQ_Flag;

int WF_counter;
int num_decoded_msg;

//Apparently set when the transmitted carrier is on
int xmit_flag;

//Apparently set when a transmission is pending
int Transmit_Armned;

int ft8_xmit_counter;

int master_decoded;

uint16_t cursor_freq;
uint16_t cursor_line;
int offset_freq;
int tune_flag;

int log_flag, logging_on;



void setup(void) {

  //Get the USB serial port running before something else goes wrong
  Serial.begin(9600);
  while (!Serial) continue;
  DTRACE();
  if (CrashReport) {
    Serial.print(CrashReport);
    delay(5000);
  }

  //Use Teensy's battery-backed clock 
  //initGPS();
  setSyncProvider(getTeensy3Time);
  delay(100);

  //Turn off the transmitter
  pinMode(PIN_PTT, OUTPUT);
  digitalWrite(PIN_PTT, LOW);

  //Initialize the SD library if the card is available
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("Unable to access the SD card");
  }

  //Initialize the display
  tft.begin(HX8357D);
  tft.fillScreen(HX8357_BLACK);
  tft.setTextColor(HX8357_YELLOW);
  tft.setRotation(3);
  tft.setTextSize(2);

//Zero-out EEPROM when executed on a new Teensy (whose memory is filled with 0xff).  This prevents
//calcuation of weird transmit offset from 0xffff filled EEPROM.
#define EEPROMSIZE 4284  //Teensy 4.1
  bool newChip = true;
  for (int adr = 0; adr < EEPROMSIZE; adr++) {
    if (EEPROM.read(adr) != 0xff) newChip = false;
  }
  if (newChip) {
    Serial.print("Initializing EEPROM for new chip\n");
    EEPROMWriteInt(10, 0);  //Address 10 is offset but the encoding remains mysterious
  }

  //Initialize the SI5351 clock generator.  Idaho Edition of Pocket FT8 board uses CLKIN input.
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 25000000, 0);          //Charlie's correction was 2200 if that someday matters
  si5351.set_pll_input(SI5351_PLLA, SI5351_PLL_INPUT_CLKIN);  //KQ7B V1.00 uses cmos CLKIN, not a XTAL
  si5351.set_pll_input(SI5351_PLLB, SI5351_PLL_INPUT_CLKIN);  //All PLLs using CLKIN
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.set_freq(3276800, SI5351_CLK2);  //Receiver
  si5351.output_enable(SI5351_CLK2, 1);

  // Gets and sets the Si47XX I2C bus address
  int16_t si4735Addr = si4735.getDeviceI2CAddress(PIN_RESET);
  if (si4735Addr == 0) {
    Serial.println("Si473X not found!");
    Serial.flush();
    while (1) continue;
  } else {
    //DPRINTF("The Si473X I2C address is 0x%2x\n", si4735Addr);
  }

  //Read the JSON configuration file into the config structure
  File configFile = SD.open(CONFIG_FILENAME, FILE_READ);
  if (!configFile) Serial.printf("unable to open %s\n", CONFIG_FILENAME);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, configFile);
  if (error) Serial.printf("Unable to read SD config file, %s\n", CONFIG_FILENAME);
  strlcpy(config.callsign, doc["callsign"] | DEFAULT_CALLSIGN, sizeof(config.callsign));  //Station callsign
  config.frequency = doc["frequency"] | DEFAULT_FREQUENCY;
  strlcpy(config.location, doc["location"] | DEFAULT_LOCATION, sizeof(config.location));
  config.audioRecordingDuration = doc["audioRecordingDuration"] | DEFAULT_AUDIO_RECORDING_DURATION;
  config.enableAVC = doc["enableAVC"] | DEFAULT_ENABLE_AVC;
  configFile.close();

  //When debugging, print the config file
  DPRINTF("doc[callsign]=%s\n", doc["callsign"] | "?");
  DPRINTF("doc[frequency]=%u\n", doc["frequency"] | 1234);
  DPRINTF("doc[location]=%s\n", doc["location"] | "?");
  DPRINTF("doc[audioRecordingDuration]=%ul\n", doc["audioRecordingDuration"] | 1234);
  DPRINTF("doc[enableAVC]=%u\n", doc["enableAVC"] | 42);

  //Ensure configured frequency is within the hardware limitations
  if (config.frequency < MINIMUM_FREQUENCY || config.frequency > MAXIMUM_FREQUENCY) {
    config.frequency = DEFAULT_FREQUENCY;  //Override config file request with default
  }

  //Argh... copy station callsign and location from config struct to C global variables (fix someday)
  strncpy(Station_Call, config.callsign, sizeof(Station_Call));
  strncpy(Locator, config.location, sizeof(Locator));

  //Initialize the SI4735 receiver
  delay(10);
  //DPRINTF("SSB patch is loading...\n");
  et1 = millis();
  loadSSB();
  et2 = millis();
  // Serial.print("SSB patch was loaded in: ");
  // Serial.print((et2 - et1));
  // Serial.println("ms");
  delay(10);
  si4735.setTuneFrequencyAntennaCapacitor(1);  // Set antenna tuning capacitor for SW.
  delay(10);
  si4735.setSSB(MINIMUM_FREQUENCY, MAXIMUM_FREQUENCY, config.frequency, 1, USB);  //FT8 is *always* USB
  delay(10);
  currentFrequency = si4735.getFrequency();
  si4735.setVolume(50);
  display_value(360, 40, (int)currentFrequency);

  Serial.println(" ");
  Serial.println("To change Transmit Frequency Offset Touch Tu button, then: ");
  Serial.println("Use keyboard u to raise, d to lower, & s to save ");
  Serial.println(" ");


  //Turn on the receiver (Req'd for V2.0 boards)
  pinMode(PIN_RCV, OUTPUT);
  digitalWrite(PIN_RCV, HIGH);

  init_DSP();
  initalize_constants();
  AudioMemory(20);

  //If we are recording raw audio to SD, then create/open the SD file for writing
  if (config.audioRecordingDuration > 0) {
    if ((ft8Raw = SD.open(AUDIO_RECORDING_FILENAME, FILE_WRITE)) == 0) {
      Serial.printf("Unable to open %s\n", AUDIO_RECORDING_FILENAME);
    }
  }

  //Start the audio pipeline
  queue1.begin();
  start_time = millis();  //Note the RTC elapsed time when we began streaming audio

  set_startup_freq();
  delay(10);
  display_value(360, 40, (int)currentFrequency);

  receive_sequence();

  update_synchronization();
  set_Station_Coordinates(Locator);
  display_all_buttons();
  open_log_file();

  auto_sync_FT8();

}  //setup()


unsigned oldFlags = 0;

void loop() {

  //Debugging aide for the flags
  unsigned newFlags = (CQ_Flag << 2) | (Transmit_Armned << 1) | (xmit_flag);
  if (newFlags != oldFlags) {
    //DPRINTF("newFlags = 0x%x\n", newFlags);
    oldFlags = newFlags;
  }

  if (decode_flag == 0) process_data();

  if (DSP_Flag == 1) {

    process_FT8_FFT();

    //Is a transmission in-progress?
    if (xmit_flag == 1) {
      int offset_index = 5;

      //KQ7B:  Is it time to modulate the SI5351 with the transmitter's next FSK tone?
      if (ft8_xmit_counter >= offset_index && ft8_xmit_counter < 79 + offset_index) {
        set_FT8_Tone(tones[ft8_xmit_counter - offset_index]);  //Program new FT8 tone into the SI5351
      }

      ft8_xmit_counter++;  //Apparently counting tones in the FSK transmission

      //Is it time to switch from transmitting to receiving (i.e. all tones sent) at the end of our timeslot?
      if (ft8_xmit_counter == 80 + offset_index) {
        xmit_flag = 0;               //Indication that the transmission has ended
        receive_sequence();          //Switch HW from transmitting to receiving
        terminate_transmit_armed();  //Switch again then update GUI
      }
    }  //xmit_flag

    DSP_Flag = 0;
    display_time(360, 0);
    display_date(360, 60);
  }  //DSP_Flag

  if (decode_flag == 1) {
    num_decoded_msg = ft8_decode();
    master_decoded = num_decoded_msg;
    decode_flag = 0;

    //Following a receive timeslot, if a message is ready for transmission,
    //turn-on the carrier and set the xmit_flag to modulate it.
    if (Transmit_Armned == 1) setup_to_transmit_on_next_DSP_Flag();
  }

  update_synchronization();
  // rtc_synchronization();

  process_touch();
  if (tune_flag == 1) process_serial();

  //If we are recording audio, then stop after the requested seconds of raw audio data
  //at 6400 samples/second.
  if ((ft8Raw != NULL) && recordSampleCount >= config.audioRecordingDuration * (unsigned)AUDIO_SAMPLE_RATE) {
    ft8Raw.close();
    ft8Raw = NULL;
    DPRINTF("Audio recording file, %s, closed with %ul samples\n", AUDIO_RECORDING_FILENAME, recordSampleCount);
  }

}  //loop()



time_t getTeensy3Time() {
  return Teensy3Clock.get();
}

void loadSSB() {
  si4735.queryLibraryId();  // Is it really necessary here? I will check it.
  si4735.patchPowerUp();
  //si4735.setPowerUp(1, 1, 1, 0, 1, SI473X_ANALOG_DIGITAL_AUDIO);
  //si4735.radioPowerUp();
  delay(50);
  si4735.downloadPatch(ssb_patch_content, size_content);
  // Parameters
  // AUDIOBW - SSB Audio bandwidth; 0 = 1.2KHz (default); 1=2.2KHz; 2=3KHz; 3=4KHz; 4=500Hz; 5=1KHz;
  // SBCUTFLT SSB - side band cutoff filter for band passand low pass filter ( 0 or 1)
  // AVC_DIVIDER  - set 0 for SSB mode; set 3 for SYNC mode.
  // AVCEN - SSB Automatic Volume Control (AVC) enable; 0=disable; 1=enable (default).
  // SMUTESEL - SSB Soft-mute Based on RSSI or SNR (0 or 1).
  // DSP_AFCDIS - DSP AFC Disable or enable; 0=SYNC MODE, AFC enable; 1=SSB MODE, AFC disable.
  //si4735.setSSBConfig(bandwidthIdx, 1, 0, 1, 0, 1);
  si4735.setSSBConfig(2, 1, 0, config.enableAVC, 0, 1);  //2 = 3 kc bandwidth
  DPRINTF("SI4735 AVC = %u\n", config.enableAVC);
}


//First step of processing received audio from queue1
void process_data() {

  if (queue1.available() >= num_que_blocks) {

    //Copy received audio from queue buffers to the FFT buffer
    for (int i = 0; i < num_que_blocks; i++) {
      copy_to_fft_buffer(input_gulp + block_size * i, queue1.readBuffer());
      queue1.freeBuffer();
    }

    for (int i = 0; i < input_gulp_size; i++) {
      dsp_buffer[i] = dsp_buffer[i + input_gulp_size];
      dsp_buffer[i + input_gulp_size] = dsp_buffer[i + 2 * input_gulp_size];
      dsp_buffer[i + 2 * input_gulp_size] = input_gulp[i];
    }

    DSP_Flag = 1;
  }
}  //process_data()


static void copy_to_fft_buffer(void *destination, const void *source) {
  const uint16_t *src = (const uint16_t *)source;
  uint16_t *dst = (uint16_t *)destination;
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
    *dst++ = *src++;  // real sample plus a zero for imaginary
  }

  //Configurable recording of raw 16-bit audio at 6400 samples/second to an SD file
  if (ft8Raw != NULL) {
    ft8Raw.write(source, AUDIO_BLOCK_SAMPLES * sizeof(uint16_t));
    recordSampleCount += AUDIO_BLOCK_SAMPLES;  //Increment count of recorded samples
    if (recordSampleCount % (unsigned)AUDIO_SAMPLE_RATE == 0) {
      DPRINTF("Audio recording in progress...\n");  //One second progress indicator
    }
  }

}  //copy_to_fft_buffer()


void rtc_synchronization() {
  getTeensy3Time();

  if (ft8_flag == 0 && second() % 15 == 0) {
    ft8_flag = 1;
    FT_8_counter = 0;
    ft8_marker = 1;
    WF_counter = 0;
  }
}

void update_synchronization() {
  current_time = millis();
  ft8_time = current_time - start_time;

  ft8_hours = (int8_t)(ft8_time / 3600000);
  hours_fraction = ft8_time % 3600000;
  ft8_minutes = (int8_t)(hours_fraction / 60000);
  ft8_seconds = (int8_t)((hours_fraction % 60000) / 1000);


  if (ft8_flag == 0 && ft8_time % 15000 <= 200) {
    ft8_flag = 1;
    FT_8_counter = 0;
    ft8_marker = 1;
    WF_counter = 0;
  }
}


void sync_FT8(void) {

  setSyncProvider(getTeensy3Time);  //commented out?
  start_time = millis();
  ft8_flag = 1;
  FT_8_counter = 0;
  ft8_marker = 1;
  WF_counter = 0;
}



void auto_sync_FT8(void) {
  DTRACE();
  // tft.setTextColor(HX8357_YELLOW, HX8357_BLACK);
  // tft.setTextSize(2);
  // tft.setCursor(0, 240);
  // tft.print("Synchronzing With RTC");
  //DPRINTF("Synchronizing with RTC\n");

  while ((second()) % 15 != 0) continue;

  start_time = millis();
  ft8_flag = 1;
  FT_8_counter = 0;
  ft8_marker = 1;
  WF_counter = 0;
  tft.setCursor(0, 260);
  tft.print("FT8 Synched With World");
}


// void initGPS() {
//   //Initialize the GPS, if any
//   SerialGPS.begin(GPS_BAUD_RATE);  //Init the GPS serial connection

//   //Try to obtain time/location from GPS
//   unsigned long age;
//   int Year;
//   byte Month, Day, Hour, Minute, Second;
//   bool gpsActive = false;
//   for (unsigned long gpsStart = millis(); millis() < gpsStart + 5000;) {
//     while (SerialGPS.available()) {

//       char c = SerialGPS.read();
//       //Serial.write(c);

//       //Wait for a message from GPS
//       if (gps.encode(c)) {  // process gps messages
//         // when TinyGPS reports new data...
//         gpsActive = true;

//         gps.crack_datetime(&Year, &Month, &Day, &Hour, &Minute, &Second, NULL, &age);
//         gps.f_get_position(&flat, &flon, &age);
//       }

//       unsigned long gpsWaitMillis = millis() - gpsStart;
//       //if (gpsWaitMillis > 15000UL) break;  //Give-up on GPS time/location

//     }  //while
//   }

//   if (gpsActive) {
//     if (age < 500) {
//       // set the MCU Time to the latest GPS reading
//       tmElements_t rtc;
//       rtc.Second = Second;
//       rtc.Minute = Minute;
//       rtc.Hour = Hour;
//       rtc.Year = Year;
//       rtc.Month = Month;
//       rtc.Day = Day;
//       uint32_t unixTime = makeTime(rtc);
//       Teensy3Clock.set(unixTime);
//       setTime(Hour,Minute,Second,Day,Month,Year);
//       DPRINTF("Teensy3Clock set\n");
//       //adjustTime(offset * SECS_PER_HOUR);
//     }

//     DPRINTF("#satellites=%d, hdop=%d, flat=%f, flon=%f, age=%ul ms\n", gps.satellites(), gps.hdop(), flat, flon, age);
//     DPRINTF("GPS Date/Time:  %u/%u/%u %u:%u:%u\n", Month, Day, Year, Hour, Minute, Second);
//   } else {
//     DPRINTF("GPS not active\n");
//   }

// }  //initGPS()