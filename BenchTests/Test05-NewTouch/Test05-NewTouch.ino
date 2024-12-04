/*
NAME
  Test05 -- Touchpad

DESCRIPTION
  Touchpad bench test.  The exercise loops, printing on the console the touchpoint's 
  X/Y location and pressure.

  If the test works correctly, the (0,0) origin will be identified on the display, and
  red dots will appear at the touchpoints.

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
  Unfortunately [2], the MCP3422 uses a 2.048 V reference while the touchpad.  To
  workaround this issue, the HW uses a 510 Ohm resistor between the X- and Y+
  connections on Teensy to reduce the maximum voltage seen by the MCP3422 to
  values within its range (<2.048V).
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
#include "MCP342x.h"

// These are the four touchscreen analog pins
#define YP 38  // must be an analog pin, use "An" notation!
#define XM 37  // must be an analog pin, use "An" notation!
#define YM 36  // can be a digital pin
#define XP 39  // can be a digital pin


// Touchpad calibrarion as investigated on V1.01 hardware
#define TS_MINX 123
#define TS_MINY 104
#define TS_MAXX 1715
#define TS_MAXY 1130


//Other touchpad constants replicated from button.cpp in PocketFT8XcvrFW
#define MINPRESSURE 120
#define PENRADIUS 3

//Build the display object using pin numbers from Charlie's Pocket FT8 code
HX8357_t3n tft = HX8357_t3n(10, 9, 8, 11, 13, 12);  //Teensy 4.1 moved SCK to dig pin 13
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 313);  //The 313 ohms is a guess for the touchpad resistance


void setup() {

  //Initialize the Arduino world and let console know we're starting
  Serial.begin(9600);
  delay(100);
  Serial.println("Starting...");

  //Initialize the display
  tft.begin(HX8357D);
  tft.fillScreen(HX8357_BLACK);
  tft.setTextColor(HX8357_YELLOW);
  tft.setRotation(3);  //PocketFT8FW uses 3
  tft.setTextSize(2);

  delay(100);

  //Identify locaton of the origin in blue
  tft.fillCircle(0, 0, PENRADIUS, HX8357_BLUE);
  tft.setCursor(0, 0);
  tft.println("(0,0)");

  //And also the opposite corner in white
  tft.fillCircle(tft.width(), tft.height(), PENRADIUS, HX8357_WHITE);

}

// the loop function runs over and over again forever
void loop() {

  //Serial.println("Loop...");

  // a point object holds x y and z coordinates
  TSPoint p = ts.getPoint();        //Touch point

  //Did we receive a valid touchpoint measurement?
  if (p.z > MINPRESSURE) {

    //Report the raw position from getPoint()
    Serial.print("X=");
    Serial.print(p.x);
    Serial.print(", Y=");
    Serial.print(p.y);
    Serial.print(", Pressure=");
    Serial.print(p.z);

    //Report the mapped position a la button.cpp in PocketFT8XcvrFW
    unsigned mappedX = map(p.x, TS_MINX, TS_MAXX, 0, 480);
    unsigned mappedY = map(p.y, TS_MINY, TS_MAXY, 0, 320);
    Serial.print("; MappedX=");
    Serial.print(mappedX);
    Serial.print(", mappedY=");
    Serial.println(mappedY);

    tft.fillCircle(mappedX, mappedY, PENRADIUS, HX8357_RED);

    //Display the touchpoint (it should lie underfinger)
    //tft.drawPixel(p.x, p.y, HX8357_YELLOW);
  }


  // delay(100);
}
