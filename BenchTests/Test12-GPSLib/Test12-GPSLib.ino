/*
NAME
  Test11 - GPS

DESCRIPTION
  Initializes GPS and prints info read from GPS using TinyGPS library

EXERCISED
  GPS
  Serial1
  TinyGPS Library 

NOTE
  The Adafruit GPS Library, not TinyGPS, is used in the production firmware.

REFERENCES



ATTRIBUTION
  Paul Stoffregen (TimeGPS.ino)
  KQ7B Adaptation for Pocket FT8

*/


#include <TimeLib.h>
#include <TinyGPS.h>  // http://arduiniana.org/libraries/TinyGPS/
#include <SoftwareSerial.h>
// TinyGPS and SoftwareSerial libraries are the work of Mikal Hart

//SoftwareSerial SerialGPS = SoftwareSerial(10, 11);  // receive on pin 10
TinyGPS gps;

//Pocket FT8 connects to GPS via Serial Port 1
#define SerialGPS Serial1

// Offset hours from gps time (UTC)
//const int offset = 1;  // Central European Time
//const int offset = -5;  // Eastern Standard Time (USA)
//const int offset = -4;  // Eastern Daylight Time (USA)
//const int offset = -8;  // Pacific Standard Time (USA)
const int offset = -7;  // Pacific Daylight Time (USA)

// Ideally, it should be possible to learn the time zone
// based on the GPS position data.  However, that would
// require a complex library, probably incorporating some
// sort of database using Eric Muller's time zone shape
// maps, at http://efele.net/maps/tz/

time_t prevDisplay = 0;  // when the digital clock was displayed

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;  // Needed for Leonardo only
  SerialGPS.begin(9600);
  Serial.println("Waiting for GPS time ... ");
}

void loop() {
  float flat, flon;
  unsigned long age;

  while (SerialGPS.available()) {

    //Wait for a message from GPS
    if (gps.encode(SerialGPS.read())) {  // process gps messages

      // when TinyGPS reports new data...
      unsigned long age;
      int Year;
      byte Month, Day, Hour, Minute, Second;
      gps.crack_datetime(&Year, &Month, &Day, &Hour, &Minute, &Second, NULL, &age);
      if (age < 500) {
        // set the MCU Time to the latest GPS reading
        setTime(Hour, Minute, Second, Day, Month, Year);
        //adjustTime(offset * SECS_PER_HOUR);
        Teensy3Clock.set(now()); // set the RTC
      }
    }
    gps.f_get_position(&flat, &flon, &age);
    //printf("#satellites=%d, hdop=%d, flat=%f, flon=%f, age=%lu ms\n", gps.satellites(), gps.hdop(), flat, flon, age);
  }
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) {  //update the display only if the time has changed
      prevDisplay = now();
      digitalClockDisplay();
    }
  }
}

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}