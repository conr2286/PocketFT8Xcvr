/**
 * Adafruit Graphics Library Font Investigation
 *
 * Objectives
 *  + Explore different font sizes
 *  + Compare typefaces
 */

#include <Arduino.h>
#include "pins.h"
#include <Adafruit_GFX.h>
#include "DEBUG.h"
#include "SPI.h"
#include "HX8357_t3n.h"

// For the Adafruit shield, these are the default.
#define TFT_DC  9
#define TFT_CS 10

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
//HX8357_t3n tft = HX8357_t3n(TFT_CS, TFT_DC,8);


#include "ili9341_t3n_font_Arial.h"
//#include "font_ArialBold.h"
// #include "font_ComicSansMS.h"
#include "ili9341_t3n_font_OpenSans.h"
//#include "ili9341_t3n_font_DroidSans.h"
// #include "font_Michroma.h"
// #include "font_Crystal.h"
// #include "font_ChanceryItalic.h"
//#include <Fonts/FreeMono9pt7b.h>
//#include <Fonts/FreeMonoBoldOblique12pt7b.h>
//#include <Fonts/FreeSerif9pt7b.h>
//#include <Fonts/FreeSans9pt7b.h>
#include "ft8_font.h"

#include "HX8357_t3n.h"

HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_RST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins

void setup() {
    char msg[] = "AB0ABC/P WA9ZXY RR73 S9 WB0AAA WA0MMM DN15";
    char symbols[256];

    for (unsigned i = 0; i < 256; i++) symbols[i] = i;

    Serial.begin(9600);
    Serial.println("Starting...");

    // Initialize the display
    //tft.fdump();
    tft.begin(30000000UL, 2000000UL);
    //tft.fdump();
    delay(1000);
    tft.setRotation(3);
    //tft.fdump();
    DPRINTF("&FT8Font=%lu, FT8Font.yAdvance=%d\n", &FT8Font, FT8Font.yAdvance);
    tft.setFont(&FT8Font);
    //tft.setFont(Arial_10);
    //tft.fdump();

    delay(1000);
    tft.fillScreen(HX8357_BLACK);
    tft.drawRect(0, 0, 480, 320,HX8357_RED);

    //tft.setClipRect(1, 1, 260, 180);
    //tft.setScrollTextArea(1, 1, 260, 180);
    //tft.enableScroll();
    tft.setCursor(0, 0);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextWrap(true);
    //DPRINTF("cx=%u, cy=%u\n", tft.getCursorX(), tft.getCursorY());
    for (int i = 0; i < 300; i+=20) {
        tft.setCursor(0, i);
        tft.print(msg);
    }

    //for (unsigned char c = '0'; c < 127; c++) tft.print(symbols[c]);

    // // Output msg
    // tft.setCursor(0, 0);
    // tft.setFont();
    // tft.setTextColor(HX8357_WHITE);
    // tft.setTextSize(2);
    // tft.print(msg);

    // for (int y = 20; y < 100; y += 20) {
    //     tft.setCursor(0, y);
    //     tft.setTextColor(HX8357_WHITE);
    //     tft.print(msg);
    // }

    // tft.setCursor(0, 40);
    // tft.setFont();
    // tft.setTextSize(2);
    // tft.print("Finished");

    Serial.println("Finished setup()\n");
}

void loop() {
    // put your main code here, to run repeatedly:
}
