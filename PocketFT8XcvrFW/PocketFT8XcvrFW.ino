

#include <Audio.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
//#include <HX8357_t3.h>
#include "HX8357_t3n.h"
#include "TouchScreen_I2C.h"
#include <MCP342x.h>
#include "si5351.h"
#include <SI4735.h>
#include "patch_full.h"  // SSB patch for whole SSBRX full download
#include <TimeLib.h>

#include "Process_DSP.h"
#include "decode_ft8.h"
#include "WF_Table.h"
#include "arm_math.h"
#include "display.h"
#include "button.h"
#include "locator.h"
#include "traffic_manager.h"

#include "Arduino.h"
#include "AudioStream.h"
#include "arm_math.h"

#include "constants.h"

#define AM_FUNCTION 1
#define RESET_PIN 20
#define PTT_Pin 13
#define USB 2


// These are the four touchscreen analog pins
#define YP 38  // must be an analog pin, use "An" notation!
#define XM 37  // must be an analog pin, use "An" notation!
#define YM 36  // can be a digital pin
#define XP 39  // can be a digital pin


HX8357_t3n tft = HX8357_t3n(10, 9, 8, 11, 14, 12);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 282);      //The 282 ohms is the measured x-Axis resistance of 3.5" Adafruit touchscreen

Si5351 si5351;
SI4735 si4735;


AudioInputAnalog adc1;  //xy=132,104
AudioRecordQueue queue1;
AudioConnection patchCord2(adc1, queue1);

q15_t dsp_buffer[3 * input_gulp_size] __attribute__((aligned(4)));
q15_t dsp_output[FFT_SIZE * 2] __attribute__((aligned(4)));
q15_t input_gulp[input_gulp_size] __attribute__((aligned(4)));

char Station_Call[] = "UrCall";  //six character call sign + /0
char Locator[] = "UrLo";         // four character locator  + /0

uint16_t currentFrequency;
long et1 = 0, et2 = 0;
const uint16_t size_content = sizeof ssb_patch_content;  // see ssb_patch_content in patch_full.h or patch_init.h;
uint32_t current_time, start_time, ft8_time;
uint32_t days_fraction, hours_fraction, minute_fraction;

uint8_t ft8_hours, ft8_minutes, ft8_seconds;
int ft8_flag, FT_8_counter, ft8_marker, decode_flag;
int WF_counter;
int num_decoded_msg;
int xmit_flag, ft8_xmit_counter, Transmit_Armned;
int DSP_Flag;
int master_decoded;

uint16_t cursor_freq;
uint16_t cursor_line;
int offset_freq;
int tune_flag;

int log_flag, logging_on;
;


void setup(void) {
  Serial.begin(9600);

  setSyncProvider(getTeensy3Time);
  delay(100);

  tft.begin(HX8357D);
  tft.fillScreen(HX8357_BLACK);
  tft.setTextColor(HX8357_YELLOW);
  tft.setRotation(3);
  tft.setTextSize(2);

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 25000000, 0);        //Charlie's correction was 2200 if that someday matters
  si5351.set_pll_input(SI5351_PLLA, SI5351_PLL_INPUT_CLKIN); //KQ7B V1.00 uses cmos CLKIN, not a XTAL
  si5351.set_pll_input(SI5351_PLLB, SI5351_PLL_INPUT_CLKIN); //All PLLs using CLKIN
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.set_freq(3276800, SI5351_CLK2);                    //Receiver  
  si5351.output_enable(SI5351_CLK2, 1);

  // Gets and sets the Si47XX I2C bus address
  int16_t si4735Addr = si4735.getDeviceI2CAddress(RESET_PIN);
  if (si4735Addr == 0) {
    Serial.println("Si473X not found!");
    Serial.flush();
    while (1)
      ;
  } else {
    Serial.print("The Si473X I2C address is 0x");
    Serial.println(si4735Addr, HEX);
  }

  delay(10);
  Serial.println("SSB patch is loading...");
  et1 = millis();
  loadSSB();
  et2 = millis();
  Serial.print("SSB patch was loaded in: ");
  Serial.print((et2 - et1));
  Serial.println("ms");
  delay(10);
  si4735.setTuneFrequencyAntennaCapacitor(1);  // Set antenna tuning capacitor for SW.
  delay(10);
  si4735.setSSB(18000, 18400, 18100, 1, USB);

  delay(10);
  currentFrequency = si4735.getFrequency();
  si4735.setVolume(50);
  Serial.print("CurrentFrequency = ");
  Serial.println(currentFrequency);
  display_value(360, 40, (int)currentFrequency);

  Serial.println(" ");
  Serial.println("To change Transmit Frequency Offset Touch Tu button, then: ");
  Serial.println("Use keyboard u to raise, d to lower, & s to save ");
  Serial.println(" ");

  pinMode(PTT_Pin, OUTPUT);
  digitalWrite(PTT_Pin, LOW);

  init_DSP();
  initalize_constants();
  AudioMemory(20);
  queue1.begin();

  start_time = millis();

  set_startup_freq();
  delay(10);
  display_value(360, 40, (int)currentFrequency);

  receive_sequence();

  update_synchronization();
  set_Station_Coordinates(Locator);
  display_all_buttons();
  open_log_file();
}



void loop() {

  if (decode_flag == 0) process_data();

  if (DSP_Flag == 1) {
    process_FT8_FFT();

    if (xmit_flag == 1) {
      int offset_index = 5;

      if (ft8_xmit_counter >= offset_index && ft8_xmit_counter < 79 + offset_index) {
        set_FT8_Tone(tones[ft8_xmit_counter - offset_index]);
      }

      ft8_xmit_counter++;

      if (ft8_xmit_counter == 80 + offset_index) {

        xmit_flag = 0;
        receive_sequence();
        terminate_transmit_armed();
      }
    }

    DSP_Flag = 0;
    display_time(360, 0);
    display_date(360, 60);
  }



  if (decode_flag == 1) {

    num_decoded_msg = ft8_decode();
    master_decoded = num_decoded_msg;
    decode_flag = 0;
    if (Transmit_Armned == 1) setup_to_transmit_on_next_DSP_Flag();
  }

  update_synchronization();
  // rtc_synchronization();
  process_touch();
  if (tune_flag == 1) process_serial();
}

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
  si4735.setSSBConfig(2, 1, 0, 1, 0, 1);  //2 = 3 kc bandwidth
}


void process_data() {


  if (queue1.available() >= num_que_blocks) {

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
}


static void copy_to_fft_buffer(void *destination, const void *source) {
  const uint16_t *src = (const uint16_t *)source;
  uint16_t *dst = (uint16_t *)destination;

  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
    *dst++ = *src++;  // real sample plus a zero for imaginary
  }
}


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

  //setSyncProvider(getTeensy3Time);
  start_time = millis();
  ft8_flag = 1;
  FT_8_counter = 0;
  ft8_marker = 1;
  WF_counter = 0;
}
