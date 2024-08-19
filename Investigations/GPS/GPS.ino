/**
  GPS --- Investigate use of a GPS for time/location

NOTES
  The code accesses the GPS module through Serial1 on Teensy pins 0 and 1 for RX1 and TX1

ATTRIBUTION
  The GPS logic for use with FT8 implemented by W5BAA
  Integration into Pocket FT8 Xcvr by KQ7B
**/

#include <TinyGPS.h>
#include "maidenhead.h"
#include "locator.h"


TinyGPS gps;
char Locator[11];  // four character locator  + /0
int auto_location;
float flat, flon;
int32_t gps_latitude;
int32_t gps_longitude;
int8_t gpsHour;
int8_t gpsMinute;
int8_t gpsSecond;
int8_t gpsHundred;
int8_t gpsOffset = 2;

bool gpsAvailable = false;


void setup() {
  Serial1.begin(9600);  //Teensy pins 0 and 1 for RX1 and TX1
}

void parse_NEMA(void) {
  while (Serial1.available()) {
    gpsAvailable = true;
    if (gps.encode(Serial1.read())) {  // process gps messages
      // when TinyGPS reports new data...
      unsigned long age;
      int Year;
      byte Month, Day, Hour, Minute, Second, Hundred;
      gps.crack_datetime(&Year, &Month, &Day, &Hour, &Minute, &Second, &Hundred, &age);
      gps.f_get_position(&flat, &flon, &age);

      Second = Second + gpsOffset;

      //setTime(Hour, Minute, Second, Day, Month, Year);
      //Teensy3Clock.set(now());  // set the RTC
      gpsHour = Hour;
      gpsMinute = Minute;
      gpsSecond = Second;
      gpsHundred = Hundred;
      char* locator = get_mh(flat, flon, 4);
      for (int i = 0; i < 11; i++) Locator[i] = locator[i];
      set_Station_Coordinates(Locator);
    }
  }
}

void loop() {
  parse_NEMA();
  if (gpsAvailable) {
    printf("%2d:%2d:%2d lat/lon %f %f %12s\n", gpsHour, gpsMinute, gpsSecond, flat, flon, Locator);
  } else {
    printf("GPS not available\n");
  }
  delay(10000);
}
