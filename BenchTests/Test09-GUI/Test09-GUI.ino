/*
NAME
  Test09-GUI coordinates the GUI's display and touchscreen components

DESCRIPTION
  This test exercises the GUI buttons.  Be sure Test05-Touchpad is working correctly
  before exercising the GUI.

  A successfull test will display touchpoint and button data, and indicate if
  executeButton() is invoked for button 0..8

EXERCISED
  + HX8357D controller and its display
  + SPI connectivity with the HX8357D
  + I2C connectivity with the MCP3422
  + MCP3422 A/D conversions
  + Analog connectivity between MCP3422 and touchpad
  + Touchpad
  + Coordinate coordination between display and touchpad
  + GUI Buttons

NOTE
  + Why Pocket FT8 used a private HX8357 library has not been investigated,
  but [1] below suggests the original Arduino library had timing issues on
  Teensy MPUs.
  + The MCP3422 instruments the resistive touchscreen using its internal
  2.048 Volt reference saturated by the PCB's 3.3 Volt supply, making
  some locations untouchable.  In the current UI, this is deemed not-a-defect
  as those areas dont have GUI controls, but it is a longer term issue.

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
#include "TouchScreen_I2C.h"
#include "MCP342x.h"
#include "button.h"

// These are the four touchscreen analog pins
#define YP 38  // must be an analog pin, use "An" notation!
#define XM 37  // must be an analog pin, use "An" notation!
#define YM 36  // can be a digital pin
#define XP 39  // can be a digital pin

//Build the display and touchscreen objects using Teensy 4.1 pin numbers
HX8357_t3n tft = HX8357_t3n(10, 9, 8, 11, 13, 12);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 282);  //The 282 ohms is the measured x-Axis resistance of 3.5" Adafruit touchscreen


void setup() {

  //Initialize the Arduino world and let console know we're starting
  Serial.begin(9600);
  Serial.println("Starting Test09-GUI...");
  delay(100);

  //Initialize the display
  tft.begin(HX8357D);
  tft.fillScreen(HX8357_BLACK);
  tft.setTextColor(HX8357_YELLOW);
  tft.setRotation(3);
  tft.setTextSize(2);

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

  //Display buttons
  display_all_buttons();
}

// the loop function runs over and over again forever
void loop() {
  //delay(100);
  process_touch();
}
