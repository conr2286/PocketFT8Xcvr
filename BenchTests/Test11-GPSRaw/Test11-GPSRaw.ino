/*
NAME
  Test11 - GPS

DESCRIPTION
  Initializes Serial1 and prints raw info read from GPS using Teensy Serial1 port

EXERCISED

NOTE
  Tested with the Adafruit Ultimate GPS (PA1616S/MTK3339 Chipset) with Teensy 4.1
  at 9600 baud on Serial1

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


void setup() {
  Serial.begin(9600);
  while (!Serial) continue;  // Needed for Leonardo only
  Serial.println("Starting... ");
}

void loop() {
  float flat, flon;
  unsigned long age;

  while (Serial1.available()) {

    char byte = Serial1.read();
    Serial.print(byte);
  }

}
