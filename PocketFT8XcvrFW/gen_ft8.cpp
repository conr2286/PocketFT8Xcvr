/*
 * gen_ft8.c
 *
 *  Created on: Oct 24, 2019
 *      Author: user
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <TimeLib.h>

#include "pack.h"
#include "encode.h"
#include "constants.h"

#include "gen_ft8.h"

#include <stdio.h>

#include "DEBUG.h"

//#include <HX8357_t3.h>
#include "HX8357_t3n.h"
extern HX8357_t3n tft;

#include "arm_math.h"
#include <string.h>
#include "decode_ft8.h"
#include "display.h"
#include "locator.h"
//#include "log_file.h"
#include "traffic_manager.h"


char Your_Call[] = "W5XXX";
char Your_Locator[] = "AA00";

char test_station[] = "W5BAA";
char test_target[] = "W5ITU";
char test_RSL[] = "R-15";

char Target_Call[7];     //six character call sign + /0
char Target_Locator[5];  // four character locator  + /0
int Target_RSL;          // four character RSL  + /0
char CQ_Target_Call[7];

char reply_message[18];
char reply_message_list[18][8];
int reply_message_count;
char message[18];   //FT8 message text pending transmission.
int message_state;  //Non-zero => message[] is valid/ready.

extern int log_flag, logging_on;
extern time_t getTeensy3Time();

extern char Station_Call[];
extern char Locator[];
char ft8_time_string[] = "15:44:15";

int max_displayed_messages = 8;





//Constructs and displays the requested FT8 outbound message and sets the
//message_state flag indicating that message[] is valid.  Does not
//actually turn-on the transmitter.
void set_message(uint16_t index) {

  DPRINTF("set_message(%u)\n", index);

  char big_gulp[60];
  uint8_t packed[K_BYTES];
  char blank[] = "                   ";
  char seventy_three[] = "RR73";
  char Reply_State[20];

  getTeensy3Time();
  char rtc_string[10];  // print format stuff
  sprintf(rtc_string, "%2i:%2i:%2i", hour(), minute(), second());

  strcpy(message, blank);
  clear_FT8_message();

  switch (index) {

    case 0:
      sprintf(message, "%s %s %s", "CQ", Station_Call, Locator);

      break;

    case 1:
      sprintf(message, "%s %s %s", Target_Call, Station_Call, Locator);

      break;

    case 2:
      sprintf(message, "%s %s %3i", Target_Call, Station_Call, Target_RSL);

      break;

    case 3:
      sprintf(message, "%s %s %3s", Target_Call, Station_Call, seventy_three);

      break;
  }
  tft.setTextColor(HX8357_WHITE, HX8357_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 260);
  tft.print(message);

  pack77_1(message, packed);
  genft8(packed, tones);

  message_state = 1;

  //	sprintf(big_gulp,"%s %s", rtc_string, message);
  //	if (logging_on == 1) write_log_data(big_gulp);

}  //set_message()


void clear_FT8_message(void) {

  char blank[] = "                      ";

  tft.setTextColor(HX8357_YELLOW, HX8357_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 260);
  tft.print(blank);

  message_state = 0;
}

void clear_reply_message_box(void) {

  tft.fillRect(0, 100, 400, 140, HX8357_BLACK);
}
