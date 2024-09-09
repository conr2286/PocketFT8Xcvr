/*
NAME
  Test03-Display -- Exercise the display

DESCRIPTION
  This is the first test requiring the display to be plugged into
  the PocketFT8Xcvr board.  If it works, it prints "Hello world" on
  the display, exercising the display and its PCB connections
  (power, SPI, etc) to Teensy.  It also prints some diagnostics to
  the console to hopefully assist with debugging.

NOTE
  The original Pocket FT8 implementation with Teensy 3.6 had SCK on
  digital pin 14.  The Teensy 4.1 implementation required a move to
  pin 13.

EXERCISED
  + HX8357D controller and its display
  + SPI connectivity with the HX8357D

NOTE
  + Why Pocket FT8 used a private HX8357 library has not been investigated,
  but [1] below suggests the original Arduino library had timing issues on
  Teensy MPUs.

REFERENCES
  1. https://forum.pjrc.com/index.php?threads/adafruit-3-5-tft-hx8357d.28055/page-2
  2. https://github.com/adafruit/Adafruit_HX8357_Library/blob/master/examples/graphicstest/graphicstest.ino

ATTRIBUTION
  KQ7B with major leverage from [2]

*/



#include <Wire.h>
#include <SPI.h>
//#include <HX8357_t3.h>
#include "HX8357_t3n.h"

//Build the display object using Teensy 4.1 pin numbers
HX8357_t3n tft = HX8357_t3n(10, 9, 8, 11, 13, 12);


void setup() {

  //Initialize the Arduino world and let console know we're starting
  Serial.begin(9600);
  Serial.println("Starting...");
  delay(100);

  //Initialize the display
  tft.begin(HX8357D);
  tft.fillScreen(HX8357_BLACK);
  tft.setRotation(1);

  delay(100);

  // Read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(HX8357_RDMODE);
  Serial.print("Display Power Mode: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDMADCTL);
  Serial.print("MADCTL Mode: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDPIXFMT);
  Serial.print("Pixel Format: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDIMGFMT);
  Serial.print("Image Format: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x");
  Serial.println(x, HEX);

  //Output the test message to the display
  tft.setCursor(0, 0);
  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(1);
  tft.println("Hello world!");

}

// the loop function runs over and over again forever
void loop() {
  delay(100);
}
