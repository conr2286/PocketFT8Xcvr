// TFT stuff - change these defs for your specific LCD
#include <arm_math.h>
#include "WString.h"  //This defines String data type
#define BLACK HX8357_BLACK
#define WHITE HX8357_WHITE
#define RED HX8357_RED
#define GREEN HX8357_GREEN
#define YELLOW HX8357_YELLOW
#define BLUE HX8357_BLUE

#define USB_WIDE 0  // filter modes
#define USB_NARROW 1
#define LSB_WIDE 2
#define LSB_NARROW 3

void display_value(int x, int y, int value);
void erase_value(int x, int y);
void display_time(int x, int y);
void display_date(int x, int y);
void display_station_data(int x, int y);

void make_filename(void);
bool open_log_file(void);
void write_log_data(char *data);
