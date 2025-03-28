/**
 * Adafruit Graphics Library Font Investigation
 *
 * Objectives
 *  + Explore different font sizes
 *  + Compare typefaces
 */

#include <Adafruit_GFX.h>  //Note:  GFX must include prior to HX8357
#include <Arduino.h>

#include "AListBox.h"
#include "DEBUG.h"
#include "HX8357_t3n.h"
#include "SPI.h"
#include "pins.h"

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

// HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_RST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins

// AListBox box1(&tft, 0, 0, 240, 200, RED);

void setup() {
    char msg[] = "AB0ABC/P WA9ZXY RR73 S9";

    Serial.begin(9600);
    Serial.println("Starting...");

    // Initialize the display
    HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_RST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins

    tft.begin(30000000UL, 2000000UL);
    delay(1000);
    tft.setRotation(3);
    tft.setFont(&FT8Font);
    tft.fillScreen(HX8357_BLACK);
    delay(1000);

    // Build the list box1
    // AListBox box1 = AListBox(&tft, ARect(0, 100, 256, 250), RED);

    // // Simple test to get something on the display
    // box1.addItem(msg);

    // // Populate box2 with multiple items
    // AListBox box2 = AListBox(&tft, ARect(257, 100, 479, 250), RED);
    // box2.addItem("AB0ABC/P WA9ZXY RR73");
    // box2.addItem("AG0E KQ7B DN15");
    // box2.addItem("KQ7B AG0E -12");
    // box2.addItem("AG0E KQ7B -3");
    // box2.addItem("KQ7B AG0E RR73");
    // box2.addItem("AG0E KQ7B 73");

    // Try mixing printf with addItem
    AListBox box3 = AListBox(&tft, ARect(0, 151, 150, 320), RED);
    //box3.printf("Hello world\n");
    //box3.addItem("Oh... hi");
    box3.addItem("text");
    box3.printf("ooo ");
    box3.printf("bar\n");
    box3.addItem("More text");
    box3.printf("Good");
    box3.printf(" bye\n");

    // // Check out pixel placement
    // AListBox box4 __attribute__((unused)) = AListBox(&tft, 160, 252, 100, 50, WHITE);
    // tft.setClipRect();
    // tft.fillRect(161, 252, 251 - 2, 50 - 2, GREEN);

    // // Test selections
    // Serial.printf("box2.getSelection(0,0)=%d\n", box2.getSelectedItem(0, 0));
    // Serial.printf("box2.getSelection(255,105)=%u\n", box2.getSelectedItem(255, 105));

    Serial.println("Finished setup()\n");
}

void loop() {
    // put your main code here, to run repeatedly:
}
