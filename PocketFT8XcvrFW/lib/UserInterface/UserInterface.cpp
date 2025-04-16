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
#include "AScrollBox.h"         //Scrolling interactive text box
#include "APixelBox.h"        //Interactive raster box
#include "ATextBox.h"         //Non-interactive text
#include "AToggleButton.h"    //Stateful button
#include "DEBUG.h"            //USB Serial debugging on the Teensy 4.1
#include "FT8Font.h"          //Customized font for the Pocket FT8 Revisited
#include "HX8357_t3n.h"       //WARNING:  #include HX8357_t3n following Adafruit_GFX
#include "TouchScreen_I2C.h"  //MCP342X interface to Adafruit's 2050 touchscreen
#include "pins.h"             //Pocket FT8 pin assignments for Teensy 4.1 MCU

/**
 * @brief Start-up the Adafruit Display, GFX adapter and library, the resistive touchscreen, and widgets
 */
void UserInterface::begin() {
    DTRACE();

    // Define the interfaces/adapters for accessing the underlying graphics libraries and hardware
    tft = new HX8357_t3n(PIN_CS, PIN_DC, PIN_RST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // These are PFR Teensy pin assignments
    gui = new AGUI(tft, 3, &FT8Font);                                             // Graphics adapter insulation from the multitude of Adafruit GFX libraries
    ts = new TouchScreen(PIN_XP, PIN_YP, PIN_XM, PIN_YM, 282);                    // 282 ohms is the measured x-Axis resistance of my Adafruit 2050 touchscreen

    // Build the Waterfall
    theWaterfall = new Waterfall();

    // Build the stationInfo
    stationInfo = new AListBox(DateX, DateY, DateW, 107, A_DARK_GREY);
    stationInfo->setItem(0, "04/11/25", A_GREEN, A_BLACK);
    stationInfo->setItem(1, "21:44:04", A_GREEN, A_BLACK);
    stationInfo->setItem(2, "DN15", A_GREEN, A_BLACK);
    stationInfo->setItem(3, "WN8ABC/P", A_WHITE, A_BLACK);
    stationInfo->setItem(4, "7074 kHz", A_WHITE, A_BLACK);

    // Build the decoded messages box
    decodedMsgs = new AScrollBox(DecodedMsgsX, DecodedMsgsY, DecodedMsgsW, DecodedMsgsH, A_DARK_GREY);
    decodedMsgs->addItem("WN9ABC/P KA0XYZ RR73 S3");
    decodedMsgs->addItem("WN9ABC/P KA0XYZ RR73 S3");
    decodedMsgs->addItem("WN9ABC/P KA0XYZ RR73 S3");
    decodedMsgs->addItem("WN9ABC/P KA0XYZ RR73 S3");
    decodedMsgs->addItem("WN9ABC/P KA0XYZ RR73 S3");
    decodedMsgs->addItem("WN9ABC/P KA0XYZ RR73 S3");

    // Build the station messages box
    stationMsgs = new AScrollBox(StationMsgsX, StationMsgsY, StationMsgsW, StationMsgsH, A_DARK_GREY);
    stationMsgs->addItem("CQ WN8ABC DN15");
    stationMsgs->addItem("WN8ABC KA0XYZ DN14");
    stationMsgs->addItem("KA0XYZ WN8ABC -5");

    // Application message box
    appMessage = new ATextBox("Logged #42",AppMsgX, AppMsgY, AppMsgW, AppMsgH, A_DARK_GREY);

    // Build the buttons
    b0 = new AToggleButton("CQ", 0 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);
    b1 = new AToggleButton("Ab", 1 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);
    b2 = new AToggleButton("Tu", 2 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);
    b3 = new AToggleButton("Tx", 3 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);
    b4 = new AToggleButton("M0", 4 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);
    b5 = new AToggleButton("M1", 5 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);
    b6 = new AToggleButton("M2", 6 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);
    b7 = new AToggleButton("Sy", 7 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);

    delay(2000);
    //stationMsgs->setItemColor(n, A_YELLOW);
}
