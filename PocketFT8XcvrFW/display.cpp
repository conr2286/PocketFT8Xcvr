
#include "HX8357_t3n.h"
#include "display.h"
#include <TimeLib.h>

#include <SD.h>
#include <SPI.h>

char log_filename[] = "FT8_Traffic.txt";
File Log_File;

extern time_t getTeensy3Time();
extern char Station_Call[];  //six character call sign + /0
extern char Locator[];       // four character locator  + /0

extern int log_flag, logging_on;


extern HX8357_t3n tft;


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
  char string[10];  // print format stuff
  sprintf(string, "%2i:%2i:%2i", hour(), minute(), second());
  tft.setTextColor(HX8357_YELLOW, HX8357_BLACK);
  tft.setTextSize(2);
  tft.setCursor(x, y);
  tft.print(string);
}

void display_date(int x, int y) {
  getTeensy3Time();
  char string[10];  // print format stuff
  sprintf(string, "%2i %2i %4i", day(), month(), year());
  tft.setTextColor(HX8357_YELLOW, HX8357_BLACK);
  tft.setTextSize(2);
  tft.setCursor(x, y);
  tft.print(string);
}



void make_filename(void) {
  getTeensy3Time();
  snprintf((char *)log_filename,sizeof(log_filename), "%2i%2i%4i%2i%2i", day(), month(), year(), hour(), minute());
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
