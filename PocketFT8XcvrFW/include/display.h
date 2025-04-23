#pragma once

#include <arm_math.h>

#include "HX8357_t3n.h"
#include "WString.h"  //This defines String data type

// Define preference to format/display date as MMDDYY or as YYMMDD on the display
#define DISPLAY_DATE MMDDYY

// Define display extent
#define DISPLAY_MAX_X 479
#define DISPLAY_MAX_Y 319

// Adafruit Graphics Library Fixed Point Text Size 2 Metrics
#define TEXT2_WIDTH 12                                    // Pixels
#define TEXT2_HEIGHT 16                                   // Pixels
#define TEXT2_LEADING 4                                   // Space between lines
#define TEXT2_LINE_HEIGHT (TEXT2_HEIGHT + TEXT2_LEADING)  // Line height including text and leading

//------Upper-right region of the display----------------------------------------------------------

// Define location of time region on display
#define DISPLAY_TIME_X 360
#define DISPLAY_TIME_Y 1

// Define location of selected (clicked) call on display
#define DISPLAY_SELECTED_X 360
#define DISPLAY_SELECTED_Y 60

// Define location of current frequency (kHz) on display
#define DISPLAY_FREQUENCY_X 360
#define DISPLAY_FREQUENCY_Y 40

// Define location of date region on display
#define DISPLAY_DATE_X 360
#define DISPLAY_DATE_Y 20

//------Decoded messages region of the display-----------------------------------------------------
#define DISPLAY_DECODED_CHARS 21                                 // #chars in a line of decoded text
#define DISPLAY_DECODED_LINES 6                                  // #lines of messages in this region
#define DISPLAY_DECODED_X 0                                      // Upper-left corner X
#define DISPLAY_DECODED_Y 100                                    // Upper-left corner Y
#define DISPLAY_DECODED_W (DISPLAY_DECODED_CHARS * TEXT2_WIDTH)  // Width in pixels
#define DISPLAY_DECODED_H 140                                    // Height in pixels

//------Calling stations region of the display-----------------------------------------------------
#define DISPLAY_CALLING_X (DISPLAY_DECODED_Y + DISPLAY_DECODED_H + TEXT2_LINE_HEIGHT)
#define DISPLAY_CALLING_Y 100

//------Outbound (info) message region of the display---------------------------------------------
#define DISPLAY_OUTBOUND_X 0
#define DISPLAY_OUTBOUND_Y 250

//------Button bar location-----------------------------------------------------------------------
#define BUTTON_BAR_X 0    // Button bar extends across the width of the screen
#define BUTTON_BAR_Y 290  // Top of button bar
#define BUTTON_BAR_H 30   // Height of button bar

//------Transmit/Receive/Pending/Tuning indicator--------------------------------------------
#define DISPLAY_XMIT_RECV_INDICATOR_X 360
#define DISPLAY_XMIT_RECV_INDICATOR_Y 80

// TFT stuff - change these defs for your specific LCD
#define BLACK HX8357_BLACK
#define WHITE HX8357_WHITE
#define RED HX8357_RED
#define GREEN HX8357_GREEN
#define YELLOW HX8357_YELLOW
#define BLUE HX8357_BLUE

// // Why are these defined in display.h???  TODO:  Prune if no longer needed.
// #define USB_WIDE 0  // filter modes
// #define USB_NARROW 1
// #define LSB_WIDE 2
// #define LSB_NARROW 3

// // Transmit/Receive/Pending indicator icon
// typedef enum {
//     INDICATOR_ICON_RECEIVE = 0,
//     INDICATOR_ICON_PENDING = 1,
//     INDICATOR_ICON_TRANSMIT = 2,
//     INDICATOR_ICON_TUNING = 3
// } IndicatorIconType;

char* strlpad(char* str, int size, char c);
//char* strlpad(char* dst, char* src, char c, unsigned size);

void display_frequency(int x, int y, int value);
// void erase_value(int x, int y);
void display_time(int x, int y);
void display_date(int x, int y);
// void display_station_data(int x, int y);
void displayInfoMsg(const char* msg);
void displayInfoMsg(const char* msg, uint16_t color);

//void setXmitRecvIndicator(IndicatorIconType indicator);