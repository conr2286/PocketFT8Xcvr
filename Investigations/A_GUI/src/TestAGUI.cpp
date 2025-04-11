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

#include "AListBox.h"
#include "APixelBox.h"
#include "ATextBox.h"
#include "AToggleButton.h"
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
    void touchItem(int item, bool selected) override {
        DPRINTF("touchItem(%d,%d)\n", item, selected);
        if (selected) {
            setItem(item, msg, BLACK, DEFAULT_SPECIAL_COLOR);
        } else {
            setItem(item, msg, fgColor, bgColor);
        }
    }
};

Box box1 = Box(ARect(0, 0, 280, 100), RED);
Box box2 = Box(ARect(0, 152, 280, 100), RED);

class Button : public AToggleButton {
   public:
    Button(const char* str, ACoord x1, ACoord y1, ACoord w, ACoord h, bool border = true) : AToggleButton(str, x1, y1, w, h, border) { DTRACE(); }
    void touchButton() {
        DPRINTF("touchButton()\n");
    }
};

Button b1 = Button("CQ", 0, 290, 42, 29);
Button b2 = Button("X", 45, 290, 42, 29, false);

class Raster : public APixelBox {
   public:
    Raster(ACoord x1, ACoord y1, APixelPos nRows, ACoord nCols) : APixelBox(x1, y1, nRows, nCols) { DTRACE(); }
    void touchPixel(APixelPos row, APixelPos col) {
        DTRACE();
        drawPixel(row, col, YELLOW);
    }
};

Raster r1 = Raster(290, 0, 150, 150);

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
    delay(1000);
    AWidget::processTouch(10, 10);
    delay(1000);
    AWidget::processTouch(10, 10);

    // Populate box2 with multiple items
    box2.addItem("AB0ABC/P WA9ZXY RR73");
    box2.addItem("AG0E KQ7B DN15");
    box2.addItem("KQ7B AG0E -12");
    box2.addItem("AG0E KQ7B -3");
    box2.addItem("KQ7B AG0E RR73");
    box2.addItem("AG0E KQ7B 73");

    delay(1000);
    AWidget::processTouch(10, 300);  // On
    delay(2000);
    AWidget::processTouch(10, 300);  // Off
    delay(2000);
    AWidget::processTouch(50, 300);  // On
    delay(2000);
    AWidget::processTouch(50, 300);  // On

    for (APixelPos n = 0; n < 300; n++) {
        r1.drawPixel(n, 50, WHITE);
        r1.drawPixel(100, n, BLUE);
        r1.drawPixel(n, n, GREEN);
    }

    AWidget::processTouch(300, 100);
    delay(2000);

    ATextBox* txt1 = new ATextBox("ATextBox", 290, 155, 100, AGUI::getLeading());
    ATextBox* txt2 = new ATextBox("No border", 290, 180, 100, AGUI::getLeading(), false);

    txt1->repaintWidget();
    txt2->repaintWidget();

    delay(2000);
    Serial.println("Finished setup()\n");
}

void loop() {
    // put your main code here, to run repeatedly:
}
