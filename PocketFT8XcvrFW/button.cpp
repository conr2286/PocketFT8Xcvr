
#include "DEBUG.h"
#include "HX8357_t3n.h"
#include "button.h"
#include "display.h"
#include "gen_ft8.h"
#include "traffic_manager.h"
#include "decode_ft8.h"
//#include <Encoder.h>
#include "Process_DSP.h"
#include <EEPROM.h>
#include "TouchScreen_I2C.h"
#include <SI4735.h>
#include <Wire.h>
#include "pins.h"

//Define which I2C bus we are using
#define WIRE WIRE_ETC

// This is calibration data for the raw touch data to the screen coordinates
// using 510 Ohm resistors to reduce the driven voltage to Y+ and X-
#define TS_MINX 123
#define TS_MINY 104
#define TS_MAXX 1715
#define TS_MAXY 1130

#define MINPRESSURE 120
#define PENRADIUS 3

extern HX8357_t3n tft;
extern TouchScreen ts;

extern int Transmit_Armned;

extern SI4735 si4735;
extern uint16_t currentFrequency;
#define USB 2

uint16_t draw_x, draw_y, touch_x, touch_y;
int test;

extern int master_decoded;
extern void sync_FT8(void);
extern uint16_t cursor_freq;
extern int tune_flag;
extern uint16_t cursor_line;
extern int offset_freq;
#define ft8_shift 6.25
int start_up_offset_freq;
extern int log_flag, logging_on;

int CQ_Flag;
int Beacon_State;

#define numButtons 9
#define button_height 30
#define button_line 290
#define button_width 42

long positionRight = 500;
uint32_t last_touch;
TSPoint pi, pw;  //

typedef struct
{
  const char* text;

  bool state;
  const uint16_t x;
  const uint16_t y;
  const uint16_t w;
  const uint16_t h;
  bool active_state;
} ButtonStruct;




static ButtonStruct sButtonData[] = {
  { /*text*/ "CQ",  //button 0
    /*state*/ false,
    /*x*/ 0,
    /*y*/ button_line,
    /*w*/ button_width,
    /*h*/ button_height },

  { /*text*/ "Lo",  //button 1
    /*state*/ false,
    /*x*/ 53,
    /*y*/ button_line,
    /*w*/ button_width,
    /*h*/ button_height },

  { /*text*/ "RS",  //button 2
    /*state*/ false,
    /*x*/ 106,
    /*y*/ button_line,
    /*w*/ button_width,
    /*h*/ button_height },

  { /*text*/ "73",  //button 3
    /*state*/ false,
    /*x*/ 159,
    /*y*/ button_line,
    /*w*/ button_width,
    /*h*/ button_height },

  { /*text*/ "Cl",  //button 4
    /*state*/ false,
    /*x*/ 212,
    /*y*/ button_line,
    /*w*/ button_width,
    /*h*/ button_height },

  { /*text*/ "Tu",  //button 5
    /*state*/ false,
    /*x*/ 265,
    /*y*/ button_line,
    /*w*/ button_width,
    /*h*/ button_height },

  { /*text*/ "Tx",  //button 6
    /*state*/ false,
    /*x*/ 318,
    /*y*/ button_line,
    /*w*/ button_width,
    /*h*/ button_height },

  { /*text*/ "Lg",  //button 7
    /*state*/ false,
    /*x*/ 371,
    /*y*/ button_line,
    /*w*/ button_width,
    /*h*/ button_height },

  { /*text*/ "Sy",  //button 8
    /*state*/ false,
    /*x*/ 424,
    /*y*/ button_line,
    /*w*/ button_width,
    /*h*/ button_height }


};  // end of button definition


void drawButton(uint16_t i) {
  uint16_t color;
  if (sButtonData[i].state)
    color = HX8357_RED;
  else
    color = HX8357_GREEN;

  tft.drawRect(sButtonData[i].x, sButtonData[i].y, sButtonData[i].w, sButtonData[i].h, color);
  tft.setCursor(sButtonData[i].x + 7, sButtonData[i].y + 7);
  tft.setTextColor(HX8357_WHITE);
  tft.print(sButtonData[i].text);
}



void display_all_buttons(void) {
  drawButton(0);
  drawButton(1);
  drawButton(2);
  drawButton(3);
  drawButton(4);
  drawButton(5);
  drawButton(6);
  drawButton(7);
  drawButton(8);

  for (int i = 0; i < numButtons; i++) sButtonData[i].active_state = true;
}


void checkButton(void) {

  for (uint8_t i = 0; i < numButtons; i++) {
    if (testButton(i) == 1 && sButtonData[i].active_state) {
      sButtonData[i].state = !sButtonData[i].state;
      drawButton(i);
      executeButton(i);
    }
  }
  //DTRACE();
  //DPRINTF("CQ_Flag=%u\n", CQ_Flag);
}

int button_delay = 100;

void executeButton(uint16_t index) {
  int Idx = 0;
  DPRINTF("executeButton(%u)\n", index);
  switch (index) {

    case 0:  //CQ (e.g. CQ KQ7B DN15)
      //DTRACE();
      if (sButtonData[0].state) {
        //DTRACE();
        CQ_Flag = 1;
        sButtonData[6].active_state = false;
        Beacon_State = 0;
      } else {
        //DTRACE();
        CQ_Flag = 0;
        sButtonData[6].active_state = true;
      }
      delay(button_delay);
      break;

    case 1:  //Lo --- Location Msg (e.g. AG0E KQ7B DN15)
      set_message(1);
      sButtonData[1].state = true;
      drawButton(1);
      delay(button_delay);
      sButtonData[1].state = false;
      drawButton(1);
      break;

    case 2:  //Rs --- Received Signal Msg (e.g. AG0E KQ7B -3)
      set_message(2);
      sButtonData[2].state = true;
      drawButton(2);
      delay(button_delay);
      sButtonData[2].state = false;
      drawButton(2);
      break;

    case 3:  //73 --- 73 Msg (e.g. AG0E KQ7B RR73)
      set_message(3);
      sButtonData[3].state = true;
      drawButton(3);
      delay(button_delay);
      sButtonData[3].state = false;
      drawButton(3);
      break;

    case 4:  //Cl --- Clears the FT8 message buffer
      clear_FT8_message();
      sButtonData[4].state = true;
      drawButton(4);
      delay(button_delay);
      sButtonData[4].state = false;
      drawButton(4);
      break;

    case 5:  //Tu --- Toggle tune on/off
      if (sButtonData[5].state) {
        tune_On_sequence();
        tune_flag = 1;
        delay(button_delay);
      } else {
        tune_Off_sequence();
        tune_flag = 0;
        delay(button_delay);
      }
      break;

    case 6:  //Tx
      if (sButtonData[6].state) {
        Transmit_Armned = 1;
        delay(button_delay);
      } else {
        Transmit_Armned = 0;
        delay(button_delay);
      }
      break;

    case 7:  //Lg
      if (log_flag == 1 && sButtonData[7].state) {

        logging_on = 1;
        sButtonData[7].state = true;
        drawButton(7);
        delay(button_delay);

      } else {
        logging_on = 0;
        sButtonData[7].state = false;
        drawButton(7);
        delay(button_delay);
      }
      break;

    case 8:  //SY
      sButtonData[8].state = true;
      drawButton(8);
      delay(button_delay);
      sync_FT8();
      sButtonData[8].state = false;
      drawButton(8);
      delay(button_delay);
      break;

    case 9:
      /*
            sButtonData[9].state = true;
            drawButton(9);
            delay(button_delay );
            store_encoders();
            sButtonData[9].state = false;
            drawButton(9);
            */


      if (sButtonData[9].state) {
        /*
            si4735.setSSB(14000, 14400, 14074, 1, USB);
            //si4735.setSSB(18000, 18400, 18100, 1, USB);
            currentFrequency = si4735.getFrequency();
            offset_freq = -150;
            display_value(360,40,(int)currentFrequency); 
            LPF_set_lpf(14);
            delay(button_delay );
          }
           else
         {
            //si4735.setSSB(14000, 14400, 14074, 1, USB);
            si4735.setSSB(7000, 7400, 7100, 1, USB);
            currentFrequency = si4735.getFrequency();
            offset_freq = -244;
            display_value(360,40,(int)currentFrequency);
            LPF_set_lpf(7);
            delay(button_delay );
            */
      }
      break;
  }

}  //execute_button()


void terminate_transmit_armed(void) {

  Transmit_Armned = 0;
  receive_sequence();
  sButtonData[6].state = false;
  drawButton(6);
}

int testButton(uint8_t index) {

  if ((draw_x > sButtonData[index].x) && (draw_x < sButtonData[index].x + sButtonData[index].w) && (draw_y > sButtonData[index].y) && (draw_y < sButtonData[index].y + sButtonData[index].h)) return 1;

  else

    return 0;
}

void check_FT8_Touch(void) {

  int FT_8_TouchIndex;
  int y_test;


  if (draw_x < 400 && (draw_y > 90 && draw_y < 300)) {
    y_test = draw_y - 90;
    FT_8_TouchIndex = y_test / 25;
    if (FT_8_TouchIndex < master_decoded) display_selected_call(FT_8_TouchIndex);
  }
}  //check_FT8_Touch()

void check_WF_Touch(void) {
  if (draw_x < 350 && draw_y < 90) {

    cursor_line = draw_x;
    cursor_freq = (uint16_t)((float)(cursor_line + ft8_min_bin) * ft8_shift);
    //DPRINTF("button.cpp:  cursor_freq=%u\n",cursor_freq);
    set_Xmit_Freq();
  }
}  //check_WF_Touch()


void set_startup_freq(void) {
  cursor_line = 100;
  start_up_offset_freq = EEPROMReadInt(10);
  cursor_freq = (uint16_t)((float)(cursor_line + ft8_min_bin) * ft8_shift);
  offset_freq = start_up_offset_freq;
  //DPRINTF("set_startup_freq:  start_up_offset_freq=%d, cursor_freq=%d, offset_freq=%d\n", start_up_offset_freq, cursor_freq, offset_freq);
}  //set_startup_freq()


void process_touch(void) {

  pi = ts.getPoint();

  if (pi.z > MINPRESSURE) {
    //DTRACE();
    pw.x = map(pi.x, TS_MINX, TS_MAXX, 0, 480);
    pw.y = map(pi.y, TS_MINY, TS_MAXY, 0, 320);
    tft.fillCircle(pw.x, pw.y, PENRADIUS, HX8357_YELLOW);

    draw_x = pw.x;
    draw_y = pw.y;

    checkButton();
    check_FT8_Touch();
    check_WF_Touch();
  }
}



void process_serial(void) {

  int incoming_byte;
  if (Serial.available() > 0) {
    incoming_byte = Serial.read();
    //display_value(240,200,incoming_byte);
    if (incoming_byte == 117) offset_freq = offset_freq + 10;
    if (incoming_byte == 100) offset_freq = offset_freq - 10;
    //display_value(240, 220, ( int ) offset_freq);
    Serial.println(offset_freq);
    set_Xmit_Freq();
    if (incoming_byte == 115) {
      store_encoders();
      Serial.println("offset_freq stored");
    }
  }
}



void store_encoders(void) {

  EEPROMWriteInt(10, offset_freq);
  delay(button_delay);
  test = EEPROMReadInt(10);
}



void EEPROMWriteInt(int address, int value) {
  uint16_t internal_value = 32768 + value;

  byte byte1 = internal_value >> 8;
  byte byte2 = internal_value & 0xFF;
  EEPROM.write(address, byte1);
  EEPROM.write(address + 1, byte2);
}

int EEPROMReadInt(int address) {
  uint16_t byte1 = EEPROM.read(address);
  uint16_t byte2 = EEPROM.read(address + 1);
  uint16_t internal_value = (byte1 << 8 | byte2);

  return (int)internal_value - 32768;
}

#define IOEXP16_ADDR 0x24

void LPF_SendRegister(uint8_t reg, uint8_t val) {
  WIRE.begin();
  WIRE.beginTransmission(IOEXP16_ADDR);
  WIRE.write(reg);
  WIRE.write(val);
  WIRE.endTransmission();
}

void LPF_init() {
  LPF_SendRegister(0x06, 0xff);
  LPF_SendRegister(0x07, 0xff);
  LPF_SendRegister(0x02, 0x00);
  LPF_SendRegister(0x06, 0x00);
  LPF_SendRegister(0x03, 0x00);
  LPF_SendRegister(0x07, 0x00);
}  //IO0, IO1 as input, IO0 to 0, IO0 as output, IO1 to 0, IO1 as output

void LPF_write(uint16_t data) {
  LPF_SendRegister(0x06, 0xff);
  LPF_SendRegister(0x07, 0xff);
  LPF_SendRegister(0x02, data);
  LPF_SendRegister(0x06, 0x00);
  LPF_SendRegister(0x03, data >> 8);
  LPF_SendRegister(0x07, 0x00);
}  // output port cmd: write bits D15-D0 to IO1.7-0.0

void LPF_set_latch(uint8_t io, bool latch) {  // reset all latches and set latch k to corresponding GPIO, all relays share a common (ground) GPIO
#define LATCH_TIME 30                         // set/reset time latch relay
  if (latch) {
    LPF_write((1U << io) | 0x0000);
    delay(LATCH_TIME);
    LPF_write(0x0000);  // set latch wired to io port
  } else {
    if (io == 0xff) {
      LPF_init();
      for (int io = 0; io != 16; io++) LPF_set_latch(io, latch);
    }  // reset all latches
    else {
      LPF_write((~(1U << io)) | 0x0002);
      delay(LATCH_TIME);
      LPF_write(0x0000);
    }  // reset latch wired to io port
  }
}

uint8_t prev_lpf_io = 0xff;  // inits and resets all latches

void LPF_set_lpf(uint8_t f) {
  uint8_t lpf_io = (f > 12) ? 3 : (f > 8) ? 5
                                : (f > 6) ? 7
                                : (f > 4) ? 9
                                          : /*(f > 2)*/ 11;  // cut-off freq in MHz to IO port of LPF relay
  if (prev_lpf_io != lpf_io) {
    LPF_set_latch(prev_lpf_io, false);
    LPF_set_latch(lpf_io, true);
    prev_lpf_io = lpf_io;
  };  // set relay
}
