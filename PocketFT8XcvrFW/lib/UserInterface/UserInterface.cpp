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
#include "AScrollBox.h"       //Scrolling interactive text box
#include "ATextBox.h"         //Non-interactive text
#include "AToggleButton.h"    //Stateful button
#include "DEBUG.h"            //USB Serial debugging on the Teensy 4.1
#include "FT8Font.h"          //Customized font for the Pocket FT8 Revisited
#include "HX8357_t3n.h"       //WARNING:  #include HX8357_t3n following Adafruit_GFX
#include "TouchScreen_I2C.h"  //MCP342X interface to Adafruit's 2050 touchscreen
#include "pins.h"             //Pocket FT8 pin assignments for Teensy 4.1 MCU

class AppScrollBox : public AScrollBox {
   public:
    AppScrollBox(ACoord x, ACoord y, ALength w, ALength h, AColor c) : AScrollBox(x, y, w, h, c) {
    }
    AScrollBoxItem* addItem(AScrollBox* pBox, String str) {
        return addItem(pBox, str);
    }
    void touchItem(AScrollBoxItem* pItem) {
        DTRACE();
        pItem->setColors(A_BLACK, A_YELLOW);
    }
};

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
    //theWaterfall = new Waterfall();

    // Build the stationInfo
    stationInfo = new AScrollBox(DateX, DateY, DateW, 107, A_DARK_GREY);
    itemDate = stationInfo->addItem(stationInfo, "XX/XX/XX",A_RED);
    itemTime = stationInfo->addItem(stationInfo, "XX:XX:XX",A_RED);
    itemLocator = stationInfo->addItem(stationInfo, "DN15",A_RED);
    itemCallsign = stationInfo->addItem(stationInfo, "NOCALL");
    itemFrequency = stationInfo->addItem(stationInfo, "NNNNN kHz");

    // Build the decoded messages box
    //decodedMsgs = new AppScrollBox(DecodedMsgsX, DecodedMsgsY, DecodedMsgsW, DecodedMsgsH, A_DARK_GREY);
    // decodedMsgs->addItem(decodedMsgs, "WN1ABC/P KA1XYZ RR73 S3");
    // decodedMsgs->addItem(decodedMsgs, "WN2ABC/P KA0XYZ RR73 S3");
    // decodedMsgs->addItem(decodedMsgs, "WN3ABC/P KA0XYZ RR73 S3");
    // decodedMsgs->addItem(decodedMsgs, "WN4ABC/P KA0XYZ RR73 S3");
    // decodedMsgs->addItem(decodedMsgs, "WN5ABC/P KA0XYZ RR73 S3");
    // decodedMsgs->addItem(decodedMsgs, "WN6ABC/P KA0XYZ RR73 S3");
    // decodedMsgs->addItem(decodedMsgs, "KQ7B AG0E DM14");
    // delay(1000);
    // AWidget::processTouch(DecodedMsgsX + 2, DecodedMsgsY + 2);

    // Build the station messages box
    //stationMsgs = new AScrollBox(StationMsgsX, StationMsgsY, StationMsgsW, StationMsgsH, A_DARK_GREY);
    // AScrollBoxItem* pCQ = stationMsgs->addItem(stationMsgs, "CQ WN8ABC DN15");
    // stationMsgs->addItem(stationMsgs, "WN8ABC KA0XYZ DN14");
    // stationMsgs->addItem(stationMsgs, "KA0XYZ WN8ABC -5");
    // pCQ->setColors(A_YELLOW, A_BLACK);

    // // Application message box
    // appMessage = new ATextBox("Logged #42", AppMsgX, AppMsgY, AppMsgW, AppMsgH, A_DARK_GREY);

    // // Build the buttons
    // b0 = new AToggleButton("CQ", ButtonX + 0 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);
    // b1 = new AToggleButton("Ab", ButtonX + 1 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);
    // b2 = new AToggleButton("Tu", ButtonX + 2 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);
    // b3 = new AToggleButton("Tx", ButtonX + 3 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);
    // b4 = new AToggleButton("M0", ButtonX + 4 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);
    // b5 = new AToggleButton("M1", ButtonX + 5 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);
    // b6 = new AToggleButton("M2", ButtonX + 6 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);
    // b7 = new AToggleButton("Sy", ButtonX + 7 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight);

    delay(2000);
}
