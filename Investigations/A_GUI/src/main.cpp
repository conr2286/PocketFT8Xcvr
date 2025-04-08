/**
 * Adafruit Graphics Library Font Investigation
 *
 * Objectives
 *  + Explore different font sizes
 *  + Compare typefaces
 */

#include <Adafruit_GFX.h>  //Note:  GFX must include prior to HX8357
#include <Arduino.h>
#include <SPI.h>

#include "AButton.h"
#include "AListBox.h"
#include "DEBUG.h"
#include "HX8357_t3n.h"
#include "SPI.h"
#include "pins.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

// #include "ili9341_t3n_font_Arial.h"
//  #include "font_ArialBold.h"
//   #include "font_ComicSansMS.h"
// #include "ili9341_t3n_font_OpenSans.h"
//  #include "ili9341_t3n_font_DroidSans.h"
//   #include "font_Michroma.h"
//   #include "font_Crystal.h"
//   #include "font_ChanceryItalic.h"
// #include <Fonts/FreeMonoBoldOblique12pt7b.h>
// #include <Fonts/FreeSerif9pt7b.h>
// #include <Fonts/FreeSans9pt7b.h>
#include "AGUI.h"
#include "ft8_font.h"

static HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_RST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Build Adafruit's HX8357 display object
static AGUI agd(&tft, 3, &FT8Font);                                                         // Build hte app's GUI object

class Box : public AListBox {
   public:
    Box(ARect rect, AColor c) : AListBox(rect, c) { DTRACE(); };
    void doTouchItem(int item) override {
        DPRINTF("doTouchItem(%d)\n", item);
    }
};

Box box1 = Box(ARect(0, 0, 280, 100), RED);
Box box2 = Box(ARect(0, 152, 280, 250), RED);

AButton b1 = AButton("CQ", 0, 290, 42, 30);



void setup() {
    char msg[] = "NW8ABC/P WA9ZXY RR73 S9";
    // char msg[] = "0";
    //  char msg[] = "$%&()?[]()@ABCDEKLMNOQRYZ0123456789,./";

    Serial.begin(9600);
    Serial.println("Starting...");

    // Build the list box1
    // DPRINTF("&tft=%lu\n", &tft);
    DTRACE();
    box1.addItem(msg);
    box1.addItem(msg);
    box1.addItem(msg);
    box1.addItem(msg);
    box1.addItem(msg);
    AWidget::processTouch(10, 10);

    // Populate box2 with multiple items
    box2.addItem("AB0ABC/P WA9ZXY RR73");
    box2.addItem("AG0E KQ7B DN15");
    box2.addItem("KQ7B AG0E -12");
    box2.addItem("AG0E KQ7B -3");
    box2.addItem("KQ7B AG0E RR73");
    box2.addItem("AG0E KQ7B 73");

    long m0 = micros();
    for (int i = 0; i < 100; i++) {
        box1.setItem(3, "test", GREEN, BLACK);
    }
    long m1 = micros();
    DPRINTF( "time(setItem) = %f uS\n", (m1 - m0) / 100.0);

    delay(5000);
    Serial.println("Finished setup()\n");
}

void loop() {
    // put your main code here, to run repeatedly:
}
