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

#include "AGUI.h"
#include "ft8_font.h"

static HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_RST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Build Adafruit's HX8357 display object
static AGUI app(&tft, 3, &FT8Font);                                                         // Build hte app's GUI object

char msg[] = "$%&()?[]()@ABCDEKLMNOQRYZ_0123456789,./";

class Box : public AListBox {
   public:
    Box(ARect rect, AColor c) : AListBox(rect, c) { DTRACE(); };
    void touchedItem(int item, bool selected) override {
        DPRINTF("touchedItem(%d,%d)\n", item, selected);
        if (selected) {
            setItem(item, msg, BLACK, DEFAULT_SPECIAL_COLOR);
        } else {
            setItem(item, msg, fgColor, bgColor);
        }
    }
};

Box box1 = Box(ARect(0, 0, 280, 100), RED);
Box box2 = Box(ARect(0, 152, 280, 250), RED);

class Button : public AToggleButton {
   public:
    Button(const char *str, ACoord x1, ACoord y1, ACoord w, ACoord h) : AToggleButton(str, x1, y1, w, h) { DTRACE(); }
    void touchedButton() {
        DPRINTF("touchedButton()\n");
    }
};

Button b1 = Button("CQ", 0, 290, 42, 29);

void setup() {
    // char msg[] = "NW8ABC/P WA9ZXY RR73 S9";
    //  char msg[] = "0";

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
    delay(5000);
    AWidget::processTouch(10, 10);
    delay(2000);
    AWidget::processTouch(10, 10);

    // Populate box2 with multiple items
    box2.addItem("AB0ABC/P WA9ZXY RR73");
    box2.addItem("AG0E KQ7B DN15");
    box2.addItem("KQ7B AG0E -12");
    box2.addItem("AG0E KQ7B -3");
    box2.addItem("KQ7B AG0E RR73");
    box2.addItem("AG0E KQ7B 73");

    delay(1000);
    AWidget::processTouch(10, 300); //On
    delay(2000);
    AWidget::processTouch(10, 300); //Off
    delay(5000);
    Serial.println("Finished setup()\n");
}

void loop() {
    // put your main code here, to run repeatedly:
}
