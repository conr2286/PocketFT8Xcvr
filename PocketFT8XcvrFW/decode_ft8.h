/*
 * decode_ft8.h
 *
 *  Created on: Nov 2, 2019
 *      Author: user
 */

#ifndef DECODE_FT8_H_
#define DECODE_FT8_H_


int ft8_decode(void);

typedef struct
{
  char field1[14];       //Their station's call
  char field2[14];       //Our station's call
  char field3[7];        //Extra info
  char locator[7];       //Their locator if we have it
  int freq_hz;           //
  char decode_time[10];  //Timestamp when message successfully decoded
  int sync_score;        //
  int snr;               //Their received signal level
  int distance;          //Between their and our station

} Decode;

typedef struct
{
  char decode_time[10];
  char call[7];

} Calling_Station;

typedef struct
{
  char decode_time[10];
  char call[7];
  int distance;
  int snr;
  int freq_hz;
} CQ_Station;



void save_Answer_CQ_List(void);

void display_Answer_CQ_Items(void);

int Check_Calling_Stations(int num_decoded);
void clear_CQ_List_box(void);
void display_details(int decoded_messages);
void display_messages(int decoded_messages);
void clear_display_details(void);
int Check_CQ_Calling_Stations(int num_decoded, int reply_state);
int Check_QSO_Calling_Stations(int num_decoded, int reply_state);
void Check_CQ_Stations(int num_decoded);

void display_selected_call(int index);
void process_selected_CQ(void);
#endif /* DECODE_FT8_H_ */
