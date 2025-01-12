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

#include "DEBUG.h"

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

/**
 * Retrieves the outbound message char[] string
 *
 * @return pointer to the outbound message char[] string
 *
**/
char* get_message() {
  return message;
}

/**
 *  Builds an outbound FT8 message[] for later transmission
 *  
 *  Constructs and displays the specified FT8 outbound message and sets the
 *  message_state flag indicating message[] is valid.  Does not actually
 *  start the transmitter.
 *
 *  Global variable usage includes:
 *    Station_Call[] -- Our station's callsign
 *    Locator[] --      Our station's maidenhead gridsquare
 *    TargetCall --     Worked station's callsign
 *    TargetRSL --      Worked station's Received Signal Level
 *    message[] --      The outbound message is constructed here
 *    message_state --  Status of message[]: 0==Invalid, 1==Valid
 *
 *  @param index Specifies the outbound FT8 message type, e.g.
 *              0 -- CQ:  CQ KQ7B DN15
 *              1 -- Lo:  AG0E KQ7B DN15
 *              2 -- Rs:  AG0E KQ7B -12
 *              3 -- 73:  AG0E KQ7B RR73
**/
void set_message(uint16_t index) {
  DPRINTF("set_message(%u)\n", index);

  char big_gulp[60];
  uint8_t packed[K_BYTES];
  char blank[] = "                   ";
  char seventy_three[] = "RR73";
  char Reply_State[20];

  getTeensy3Time();
  char rtc_string[10];  // print format stuff
  snprintf(rtc_string, sizeof(rtc_string), "%2i:%2i:%2i", hour(), minute(), second());

  strlcpy(message, blank,sizeof(message));
  clear_FT8_message();

  switch (index) {

    case 0:  //We are calling CQ from our Locator, e.g. CQ KQ7B DN15
      snprintf(message, sizeof(message), "%s %s %s", "CQ", Station_Call, Locator);

      break;

    case 1:  //We are calling target from our Locator, e.g. AG0E KQ7B DN15
      snprintf(message, sizeof(message), "%s %s %s", Target_Call, Station_Call, Locator);

      break;

    case 2:  //We are responding to target with their signal report, e.g. AG0E KQ7B -12
      snprintf(message, sizeof(message), "%s %s %3i", Target_Call, Station_Call, Target_RSL);

      break;

    case 3:  //We are responding to target with 73, e.g. AG0E KQ7B RR73
      snprintf(message, sizeof(message), "%s %s %3s", Target_Call, Station_Call, seventy_three);

      break;
  }
  // tft.setTextColor(HX8357_WHITE, HX8357_BLACK);
  // tft.setTextSize(2);
  // tft.setCursor(DISPLAY_OUTBOUND_X, DISPLAY_OUTBOUND_Y);
  // tft.print(message);
  displayInfoMsg(message);

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
  tft.setCursor(DISPLAY_OUTBOUND_X, DISPLAY_OUTBOUND_Y);
  tft.print(blank);

  message_state = 0;
}


//?????
void clear_reply_message_box(void) {

  tft.fillRect(0, 100, 400, 140, HX8357_BLACK);
}
