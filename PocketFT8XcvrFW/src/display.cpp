

#include <TimeLib.h>

#include <SD.h>
#include <SPI.h>

#include "HX8357_t3n.h"
#include "display.h"
#include "GPShelper.h"

char log_filename[] = "FT8_Traffic.txt";
File Log_File;

extern time_t getTeensy3Time();
extern char Station_Call[];  //six character call sign + /0
extern char Locator[];       // four character locator  + /0

extern int log_flag, logging_on;

extern GPShelper gpsHelper;  //Public data acquired by GPShelper


extern HX8357_t3n tft;




/**
 * Helper function to pad a char[] to a specified length
 *
 * @param str  Pointer to the char[] string to pad
 * @param size sizeof(str) including the NUL terminator
 * @param c    The pad character (e.g. ' ')
 *
 * strlpad() pads the specified array containing a NUL terminated char string
 * with the specified char, c, ensuring that str remains NUL terminated.  The
 * resulting string, including the NUL terminator, will occupy no more than
 * size chars in str[].
 *
**/
 char* strlpad(char* str, unsigned size, char c) {

  const char NUL=0;
  bool paddingUnderway = false;
  int i;

  for (i=0; i<size-1; i++) {
    if (str[i]==NUL) paddingUnderway=true;
    if (paddingUnderway) {
      str[i] = c;
    }
  }

  str[size-1] = NUL;

  return str;

} //strlpad()





/**
 * Local helper function to pad a char[] to a specified length
 *
 * @param dst  Pointer to the destination char[] string
 * @param src  Pointer to the source char[] string
 * @param c    The pad character (e.g. ' ')
 * @param size sizeof(dst) including the NUL terminator
 *
 * strlpad() copies chars from src[] to dst[], padding dst[] with c to ensure
 * it's filled (including the NUL terminator).  The resulting dst string,
 * including the NUL terminator, will occupy no more than size chars in dst[].
 *
**/
static char* strlpad(char* dst, char* src, char c, unsigned size) {
  const char NUL=0;
  bool paddingUnderway = false;
  int i;

  for (i=0; i<size-1; i++) {
    if (src[i]==NUL) paddingUnderway=true;
    if (paddingUnderway) {
      dst[i] = c;
    } else {
      dst[i] = src[i];
    }
  }

  dst[size-1] = NUL;

  return dst;

} //strlpad()




void display_value(int x, int y, int value) {
  char string[7];  // print format stuff
  sprintf(string, "%6i", value);
  tft.setTextColor(HX8357_YELLOW, HX8357_BLACK);
  tft.setTextSize(2);
  tft.setCursor(x, y);
  tft.print(string);
}




void display_time(int x, int y) {
  getTeensy3Time();
  char string[13];  // print format stuff
  sprintf(string, "%02i:%02i:%02i", hour(), minute(), second());
  if (gpsHelper.validFix) {
    tft.setTextColor(HX8357_YELLOW, HX8357_BLACK);  //GPS-acquired UTC time
    //strlcat(string, " UTC", sizeof(string));
  } else {
    tft.setTextColor(HX8357_RED, HX8357_BLACK);  //Unknown zone and accuracy
  }
  tft.setTextSize(2);
  tft.setCursor(x, y);
  tft.print(string);
}

void display_date(int x, int y) {
  getTeensy3Time();
  char string[13];  // print format stuff
#if DISPLAY_DATE == MMDDYY
  sprintf(string, "%02i/%02i/%02i", month(), day(), year() % 1000);
#else
  sprintf(string, "%02i/%02i/%02i", year() % 1000, month(), day());
#endif
  if (gpsHelper.validFix) {
    tft.setTextColor(HX8357_YELLOW, HX8357_BLACK);  //GPS-acquired UTC date
    //strlcat(string, " UTC", sizeof(string));
  } else {
    tft.setTextColor(HX8357_RED, HX8357_BLACK);  //Unknown zone and accuracy
  }
  tft.setTextSize(2);
  tft.setCursor(x, y);
  tft.print(string);
}




/**
 *
 * Display informational messages in the OUTBOUND text region 
 *
 * @param msg Message to be displayed
 * 
 * Note:  Display must be initialized
 *
**/
void displayInfoMsg(const char *msg) {
  // tft.setTextColor(HX8357_YELLOW, HX8357_BLACK);
  // tft.setTextSize(2);
  // tft.setCursor(DISPLAY_OUTBOUND_X, DISPLAY_OUTBOUND_Y);
  // tft.print(msg);
  displayInfoMsg(msg, HX8357_YELLOW);
}  //displayMsg()



/**
 *
 * Display informational messages in the OUTBOUND text region 
 *
 * @param msg Message to be displayed
 * @param color Color of displayed message
 * 
 * Note:  Display must be initialized
 *
**/
void displayInfoMsg(const char *msg, uint16_t color) {
  char bfr[24];
  strlpad(bfr,msg,' ',sizeof(bfr));
  tft.setTextColor(color, HX8357_BLACK);
  tft.setTextSize(2);
  tft.setCursor(DISPLAY_OUTBOUND_X, DISPLAY_OUTBOUND_Y);
  tft.print(bfr);
}  //displayMsg()



void make_filename(void) {
  getTeensy3Time();
  snprintf((char *)log_filename, sizeof(log_filename), "%2i%2i%4i%2i%2i", day(), month(), year(), hour(), minute());
  tft.setTextColor(HX8357_YELLOW, HX8357_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 200);
  tft.print(log_filename);
}

//Opens the logfile, appends asterisk line, and closes the log file.
//Displays name of the log file.
bool open_log_file(void) {

  if (!SD.begin(BUILTIN_SDCARD)) {
    tft.setTextColor(HX8357_YELLOW, HX8357_BLACK);
    tft.setTextSize(2);
    tft.setCursor(0, 200);
    tft.print("SD Card not found");
    log_flag = 0;
    return false;
  } else {
    Log_File = SD.open("FT8_Log.txt", FILE_WRITE);
    //Log_File = SD.open(log_filename, FILE_WRITE);

    Log_File.println(" ");
    Log_File.println("**********************");

    Log_File.close();
    tft.setTextColor(HX8357_YELLOW, HX8357_BLACK);
    tft.setTextSize(2);
    tft.setCursor(0, 200);
    tft.print("FT8_Log.txt");
    log_flag = 1;
    return true;
  }
}

//Displays our station callsign and our maidenhead locator
void display_station_data(int x, int y) {
  char string[13];  // print format stuff
  sprintf(string, "%7s %4s", Station_Call, Locator);
  tft.setTextColor(HX8357_YELLOW, HX8357_BLACK);
  tft.setTextSize(2);
  tft.setCursor(x, y);
  tft.print(string);
}


//Opens the log file, appends the specified string to the logfile, and closes the file
void write_log_data(char *data) {
  Log_File = SD.open("FT8_Log.txt", FILE_WRITE);
  Log_File.println(data);
  Log_File.close();
}
