/**
 * @brief The Pocket FT8 Revisited (PFR) User Interface
 *
 * @note UserInterface.cpp attempts to consolodate all user interface decisions and objects
 * in this single location facilitating future tweeks to the hardware and design.
 */

#include "UserInterface.h"

#include <Adafruit_GFX.h>  //WARNING:  #include Adafruit_GFX prior to HX8357_t3n
#include <Arduino.h>       //We use many Arduino classes and data types

#include "AColor.h"           //AGUI colors
#include "ACoord.h"           // Screen coordinate data types
#include "AGUI.h"             //The adapter for Adafruit GFX libraries
#include "AListBox.h"         //Interactive text box
#include "APixelBox.h"        //Interactive raster box
#include "ATextBox.h"         //Non-interactive text
#include "AToggleButton.h"    //Stateful button
#include "DEBUG.h"            //USB Serial debugging on the Teensy 4.1
#include "FT8Font.h"          //Customized font for the Pocket FT8 Revisited
#include "HX8357_t3n.h"       //WARNING:  #include HX8357_t3n following Adafruit_GFX
#include "TouchScreen_I2C.h"  //MCP342X interface to Adafruit's 2050 touchscreen
#include "pins.h"             //Pocket FT8 pin assignments for Teensy 4.1 MCU

/**
 * @brief Start-up the Adafruit Display, GFX adapter and library, and the resistive touchscreen
 */
void UserInterface::begin() {
    DTRACE();

    // Define the interfaces/adapters for accessing the underlying graphics libraries and hardware
    tft = new HX8357_t3n(PIN_CS, PIN_DC, PIN_RST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // These are PFR Teensy pin assignments
    gui = new AGUI(tft, 3, &FT8Font);
    ts = new TouchScreen(PIN_XP, PIN_YP, PIN_XM, PIN_YM, 282);  // 282 ohms is the measured x-Axis resistance of my Adafruit 2050 touchscreen

    // Build the Waterfall
    waterfallBox = new Waterfall();

    // Build the infoBox
    infoBox = new AListBox(DateX, DateY, DateW, 104, A_DARK_GREY);
    infoBox->setItem(0, "04/11/25", A_GREEN, A_BLACK);
    infoBox->setItem(1, "21:44:04", A_GREEN, A_BLACK);
    infoBox->setItem(2, "DN15", A_GREEN, A_BLACK);
    infoBox->setItem(3, "WN8ABC/P", A_WHITE, A_BLACK);
    infoBox->setItem(4, "7074 kHz", A_WHITE, A_BLACK);

    // Build the decoded messages box
    decodedMsgs = new AListBox(DecodedMsgsX, DecodedMsgsY, DecodedMsgsW, DecodedMsgsH, A_DARK_GREY);
    decodedMsgs->addItem("WN9ABC/P KA0XYZ RR73 S3");
    decodedMsgs->addItem("WN9ABC/P KA0XYZ RR73 S3");
    decodedMsgs->addItem("WN9ABC/P KA0XYZ RR73 S3");
    decodedMsgs->addItem("WN9ABC/P KA0XYZ RR73 S3");
    decodedMsgs->addItem("WN9ABC/P KA0XYZ RR73 S3");
    decodedMsgs->addItem("WN9ABC/P KA0XYZ RR73 S3");

    // Build the station messages box
    stationMsgs = new AListBox(StationMsgsX, StationMsgsY, StationMsgsW, StationMsgsH, A_DARK_GREY);
    stationMsgs->addItem("CQ WN8ABC DN15");
    stationMsgs->addItem("WN8ABC KA0XYZ DN14");
    stationMsgs->addItem("KA0XYZ WN8ABC -5", A_YELLOW);

    //Build the buttons
    b1 = new AToggleButton("CQ", B1X, BTY, BTW, BTH);
    b2 = new AToggleButton("AB", B2X, BTY, BTW, BTH);
    b3 = new AToggleButton("M1", B3X, BTY, BTW, BTH);
    b4 = new AToggleButton("M2", B4X, BTY, BTW, BTH);

}
