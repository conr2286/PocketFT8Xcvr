#include <Adafruit_GFX.h>  //Note:  GFX must include prior to HX8357
#include <Arduino.h>

#include "AListBox.h"
#include "DEBUG.h"
#include "HX8357_t3n.h"
#include "SPI.h"
#include "pins.h"
#include "unity.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
// HX8357_t3n tft = HX8357_t3n(TFT_CS, TFT_DC,8);

#include "ili9341_t3n_font_Arial.h"
// #include "font_ArialBold.h"
//  #include "font_ComicSansMS.h"
#include "ili9341_t3n_font_OpenSans.h"
// #include "ili9341_t3n_font_DroidSans.h"
//  #include "font_Michroma.h"
//  #include "font_Crystal.h"
//  #include "font_ChanceryItalic.h"
// #include <Fonts/FreeMono9pt7b.h>
// #include <Fonts/FreeMonoBoldOblique12pt7b.h>
// #include <Fonts/FreeSerif9pt7b.h>
// #include <Fonts/FreeSans9pt7b.h>
#include "AGUI.h"
#include "ft8_font.h"

HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_RST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins

bool confirm(const char* msg) {
    Serial.printf("%s:  Press Y if OK, N to fail:  \n",msg);
    delay(100);
    int c;
    while ((c = Serial.read()) < 0) continue;
    return c == 'y' || c == 'Y';
}

// Test basic list boxes without borders
void test_01(void) {
    TEST_MESSAGE("Starting...");

    // Build boxes sans borders
    AListBox box1 = AListBox(&tft,0, 0, 100, 100);           // Extents sans border
    AListBox box2 = AListBox(&tft, ARect(0, 150, 100, 250));  // Rectangle sans border

    // Build boxes with borders
    AListBox box3 = AListBox(&tft, 200, 0, 100, 100, AColor::RED);           // Extents with border
    AListBox box4 = AListBox(&tft, ARect(200, 150, 300, 250), AColor::RED);  // Rectangle with border

    TEST_ASSERT(confirm("You should see 4 boxes, left sans borders, right with borders"));
    delay(5000);
}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_01);
    return UNITY_END();
}

void setup() {
    Serial.begin(9600);
    //Serial.println("Starting...");

    // Initialize the display
    // HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_RST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins

    tft.begin(30000000UL, 2000000UL);
    delay(1000);
    tft.setRotation(3);
    tft.setFont(&FT8Font);
    tft.fillScreen(HX8357_DARKGREY);
    delay(1000);
    // Run the tests
    delay(2000);
    runUnityTests();
}

void tearDown(void) {
    tft.fillScreen(HX8357_DARKGREY);
}

// loop() not actually used
void loop() {}
