/*
NAME
  Test05 -- Touchpad

DESCRIPTION
  Touchpad bench test.  The exercise loops, printing on the console the touchpoint's 
  X/Y location and pressure.

EXERCISED
  + HX8357D controller and its display
  + SPI connectivity with the HX8357D
  + I2C connectivity with the MCP3422
  + MCP3422 A/D conversions
  + Analog connectivity between MCP3422 and touchpad
  + Touchpad
  + Coordinate coordination between display and touchpad

NOTE
  + Pocket FT8 uses the MCP3422 A/D converter [1] to read the touchpad.
  Unfortunately [2], the MCP3422 uses a 2.048 V reference while the touchpad
  is supplied by a 3.3 V source restricting touch to 2/3 of the pad.
  + Touchpad initialization of 313 ohms is guessing the pad's actual resistance

REFERENCES
  1. https://github.com/Rotron/Pocket-FT8/blob/main/Project%20Manual_1.0.pdf
  2. https://oz1bxm.dk/notes/HB9TVK-Pocket-FT8%20incorrect%20TFT%20display%20connections%20in%20your%20builder%27s%20notes.pdf
  3. https://oz1bxm.dk/notes/pocket-FT8.html

ATTRIBUTION
  KQ7B and Charlie [1]

*/



#include <Wire.h>
#include <SPI.h>
//#include <HX8357_t3.h>
#include "HX8357_t3n.h"
#include "TouchScreen_I2C.h"
#include <MCP342x.h>

// These are the four touchscreen analog pins
#define YP 38  // must be an analog pin, use "An" notation!
#define XM 37  // must be an analog pin, use "An" notation!
#define YM 36  // can be a digital pin
#define XP 39  // can be a digital pin


//Build the display object using pin numbers from Charlie's Pocket FT8 code
HX8357_t3n tft = HX8357_t3n(10, 9, 8, 11, 14, 12);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 313);  //The 313 ohms is a guess for the touchpad resistance


void setup() {

  //Initialize the Arduino world and let console know we're starting
  Serial.begin(9600);
  Serial.println("Starting...");
  delay(100);

  //Initialize the display
  tft.begin(HX8357D);
  tft.fillScreen(HX8357_BLACK);
  tft.setRotation(1);
  tft.setTextColor(HX8357_YELLOW);
  tft.setTextSize(2);

  delay(100);
}

// the loop function runs over and over again forever
void loop() {

  // a point object holds x y and z coordinates
  TSPoint p = ts.getPoint();

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (p.z > ts.pressureThreshhold) {
    Serial.print("X = ");
    Serial.print(p.x);
    Serial.print("\tY = ");
    Serial.print(p.y);
    Serial.print("\tPressure = ");
    Serial.println(p.z);

    //Display the touchpoint (it should lie underfinger)
    tft.drawPixel(p.x, p.y, HX8357_YELLOW);

  }

  delay(100);
}
