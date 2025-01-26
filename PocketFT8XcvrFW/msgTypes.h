#pragma once

//Define the decoded message types
typedef enum MsgTypeValues {
  MSG_CQ = 0,      //CQ/QRZ and Locator (e.g. CQ KQ7B DN15) messagen
  MSG_LOC = 1,     //Locator (e.g. AG03 KQ7B DN15, or AG0E KQ7B R DN15) message
  MSG_RSL = 2,     //Received signal level (e.g. AG0E KQ7B -10) message
  MSG_EOT = 3,     //Received end-of-transmission (e.g. RRR, RR73 or 73) message
  MSG_BLANK = 4,   //Received non-standard (hashed callsign) with a blank field3
  MSG_FREE = 5,    //Free text in field1 (e.g. TNX BOB 73 GL)
  MSG_TELE = 6,    //Telemetry (hexadecimal digits) in field1
  MSG_UNKNOWN = 7  //Something we couldn't decode
} MsgType;