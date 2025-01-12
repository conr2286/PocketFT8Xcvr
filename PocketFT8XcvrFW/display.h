#pragma once


#include <arm_math.h>
#include "HX8357_t3n.h"
#include "WString.h"  //This defines String data type


//Define preference to format/display date as MMDDYY or as YYMMDD on the display
#define DISPLAY_DATE MMDDYY


//------Upper-right region of the display----------------------------------------------------------

//Define location of time region on display
#define DISPLAY_TIME_X 360
#define DISPLAY_TIME_Y 0

//Define location of selected (clicked) call on display
#define DISPLAY_SELECTED_X 360
#define DISPLAY_SELECTED_Y 20

//Define location of current frequency (kHz) on display
#define DISPLAY_FREQUENCY_X 360
#define DISPLAY_FREQUENCY_Y 40

//Define location of date region on display
#define DISPLAY_DATE_X 360
#define DISPLAY_DATE_Y 60

//------Calling stations region of the display-----------------------------------------------------
#define DISPLAY_CALLING_X 240
#define DISPLAY_CALLING_Y 100


//------Decoded messages region of the display-----------------------------------------------------
#define DISPLAY_DECODED_X 0
#define DISPLAY_DECODED_Y 100
#define DISPLAY_DECODED_W 256
#define DISPLAY_DECODED_H 140

//------Outbound (transmitted) message region of the display---------------------------------------
#define DISPLAY_OUTBOUND_X 0
#define DISPLAY_OUTBOUND_Y 260



// TFT stuff - change these defs for your specific LCD
#define BLACK HX8357_BLACK
#define WHITE HX8357_WHITE
#define RED HX8357_RED
#define GREEN HX8357_GREEN
#define YELLOW HX8357_YELLOW
#define BLUE HX8357_BLUE


//Why are these defined in display.h???
#define USB_WIDE 0  // filter modes
#define USB_NARROW 1
#define LSB_WIDE 2
#define LSB_NARROW 3



void display_value(int x, int y, int value);
void erase_value(int x, int y);
void display_time(int x, int y);
void display_date(int x, int y);
void display_station_data(int x, int y);
void displayInfoMsg(char* msg);
void displayInfoMsg(char* msg,uint16_t color);

void make_filename(void);
bool open_log_file(void);
void write_log_data(char *data);
