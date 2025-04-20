/**
 * @brief The Pocket FT8 Revisited (PFR) User Interface
 *
 * @note UserInterface.cpp attempts to consolodate all user interface decisions and objects
 * in this single location facilitating future tweeks to the hardware and design.
 */

#include "UserInterface.h"

#include <Adafruit_GFX.h>  //WARNING:  #include Adafruit_GFX prior to HX8357_t3n
#include <Arduino.h>       //We use many Arduino classes and data types
#include <TimeLib.h>       //Teensy time

#include "AColor.h"         //AGUI colors
#include "ACoord.h"         // Screen coordinate data types
#include "AGUI.h"           //The adapter for Adafruit GFX libraries
#include "AListBox.h"       //Interactive text box
#include "APixelBox.h"      //Interactive raster box
#include "AScrollBox.h"     //Scrolling interactive text box
#include "ATextBox.h"       //Non-interactive text
#include "AToggleButton.h"  //Stateful button
#include "DEBUG.h"          //USB Serial debugging on the Teensy 4.1
#include "FT8Font.h"        //Customized font for the Pocket FT8 Revisited
#include "GPShelper.h"      //Decorator for Adafruit_GPS library
#include "HX8357_t3n.h"     //WARNING:  #include HX8357_t3n following Adafruit_GFX
#include "Sequencer.h"
#include "TouchScreen_I2C.h"  //MCP342X interface to Adafruit's 2050 touchscreen
#include "display.h"
#include "lexical.h"  //String helpers
#include "pins.h"     //Pocket FT8 pin assignments for Teensy 4.1 MCU

HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_RST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins
TouchScreen ts = TouchScreen(PIN_XP, PIN_YP, PIN_XM, PIN_YM, 282);                   // The 282 ohms is the measured x-Axis resistance of 3.5" Adafruit touchscreen in 2024
static AGUI* gui;

// Define externals required to build the application (as opposed to unit test)
#ifndef UNIT_TEST
static Sequencer& seq = Sequencer::getSequencer();  // Get a reference to the Sequencer (RoboOp)
#endif

extern char* Station_Call;

// GPS Access
extern GPShelper gpsHelper;  // TODO:  This shouldn't be an extern :()

extern UserInterface ui;

void DecodedMsgsBox::setMsg(int index, char* msg) {
    setItem(index, msg, A_WHITE, bgColor);
}

/**
 * @brief Start-up the Adafruit Display, GFX adapter and library, the resistive touchscreen, and widgets
 */
void UserInterface::begin() {
    DTRACE();

    // Define the interfaces/adapters for accessing the underlying graphics libraries and hardware
    gui = new AGUI(&tft, 3, &FT8Font);  // Graphics adapter insulation from the multitude of Adafruit GFX libraries
    // ts = new TouchScreen(PIN_XP, PIN_YP, PIN_XM, PIN_YM, 282);                    // 282 ohms is the measured x-Axis resistance of my Adafruit 2050 touchscreen

    // Build the Waterfall
    theWaterfall = new Waterfall();

    // Build the stationInfo
    stationInfo = new AScrollBox(InfoX, InfoY, InfoW, InfoH, A_DARK_GREY);
    itemDate = stationInfo->addItem(stationInfo, "", A_RED);
    itemTime = stationInfo->addItem(stationInfo, "", A_RED);
    itemLocator = stationInfo->addItem(stationInfo, "", A_RED);
    itemCallsign = stationInfo->addItem(stationInfo, "");
    itemFrequency = stationInfo->addItem(stationInfo, "");
    itemMode = stationInfo->addItem(stationInfo, "");

    // Build the decoded messages box
    decodedMsgs = new DecodedMsgsBox(DecodedMsgsX, DecodedMsgsY, DecodedMsgsW, DecodedMsgsH, A_DARK_GREY);

    // Build the station messages box
    stationMsgs = new AScrollBox(StationMsgsX, StationMsgsY, StationMsgsW, StationMsgsH, A_DARK_GREY);

    // // Application message box
    applicationMsgs = new ATextBox("Starting", AppMsgX, AppMsgY, AppMsgW, AppMsgH, A_DARK_GREY);

    // Build the buttons
    b0 = new MenuButton("CQ", ButtonX + 0 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 0);
    b1 = new MenuButton("Ab", ButtonX + 1 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 1);
    b2 = new MenuButton("Tu", ButtonX + 2 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 2);
    b3 = new MenuButton("Tx", ButtonX + 3 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 3);
    b4 = new MenuButton("M0", ButtonX + 4 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 4);
    b5 = new MenuButton("M1", ButtonX + 5 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 5);
    b6 = new MenuButton("M2", ButtonX + 6 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 6);
    b7 = new MenuButton("Sy", ButtonX + 7 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 7);
}

/**
 * @brief Display nominal operating frequency
 * @param kHz Frequency in kHz
 * @param fg Foreground color
 */
void UserInterface::displayFrequency(unsigned kHz) {
    String s = String(kHz) + " kHz";
    itemFrequency->setItemText(s, A_GREEN);
}  // displayFrequency()

/**
 * @brief Display 4-letter Maidenhead Grid Locator
 * @param grid The locator string
 * @param fg Color
 */
void UserInterface::displayLocator(String grid, AColor fg) {
    itemLocator->setItemText(grid, fg);
}

/**
 * @brief Display UTC date
 *
 * @param forceUpdate Allows app to force an update to the display
 *
 * Displayed in green if GPS disciplined, else yellow.
 *
 * We only update the display when day() changes
 */
static int lastDay = -1;
void UserInterface::displayDate(bool forceUpdate) {
    // DTRACE();
    Teensy3Clock.get();  // Sync MCU clock with RTC
    char str[13];        // print format stuff
    AColor fg;           // Text color
    int thisDay = day();
    if ((thisDay != lastDay) || forceUpdate) {
        // We can display UTC date in two possible formatts
#if DISPLAY_DATE == MMDDYY
        snprintf(str, sizeof(str), "%02i/%02i/%02i", month(), day(), year() % 1000);  // MM:DD:YY
#else
        snprintf(str, sizeof(str), "%02i/%02i/%02i", year() % 1000, month(), day());  // YY:MM:DD
#endif
        if (gpsHelper.validGPSdata) {
            fg = A_GREEN;
        } else {
            fg = A_YELLOW;
        }
        itemDate->setItemText(String(str), fg);  // Green if GPS Disciplined
    }
}  // displayDate()

/**
 * @brief Display UTC time
 *
 * Displayed in green if GPS disciplined else yellow.
 *
 * We only update the display when the second() changes.
 */
static int lastSecond = -1;
void UserInterface::displayTime() {
    Teensy3Clock.get();  // Sync MCU clock with RTC
    char str[13];        // print format stuff
    AColor fg;
    int thisSecond = second();
    if (thisSecond != lastSecond) {
        snprintf(str, sizeof(str), "%02i:%02i:%02i", hour(), minute(), second());
        if (gpsHelper.validGPSdata) {
            fg = A_GREEN;
        } else {
            fg = A_YELLOW;
        }
        itemTime->setItemText(String(str), fg);  // Green if GPS Disciplined
    }
}  // displayTime

/**
 * @brief Display station callsign
 * @param callSign Station's callsign
 */
void UserInterface::displayCallsign(String callSign) {
    itemCallsign->setItemText(callSign, A_GREEN);
}  // displayCallSign

/**
 * @brief Displays specified String in the Station's operating mode (e.g. RECEIVE, TUNE, XMIT...)
 * @param str String to display
 * @param fg Text color
 */
void UserInterface::displayMode(String str, AColor fg) {
    itemMode->setItemText(str, fg);
}

/**
 * @brief Set the GUI's Transmit/Receive/Pending icon color
 * @param indicator Specifies what we're doing
 */
void UserInterface::setXmitRecvIndicator(IndicatorIconType indicator) {
    unsigned short color;  // Indicator icon color
    char* string;
    char paddedString[9];

    switch (indicator) {
        // We are receiving
        case INDICATOR_ICON_RECEIVE:
            color = HX8357_GREEN;
            string = "RECEIVE";
            break;
        // Transmission pending for next appropriate timeslot
        case INDICATOR_ICON_PENDING:
            color = HX8357_YELLOW;
            string = "PENDING";
            break;
        // Transmission in progress
        case INDICATOR_ICON_TRANSMIT:
            color = HX8357_RED;
            string = "TRANSMIT";
            break;
        // Tuning in progress
        case INDICATOR_ICON_TUNING:
            color = HX8357_ORANGE;
            string = "TUNING";
            break;
        // Lost in the ozone again
        default:
            color = HX8357_BLACK;
            string = " ";
            break;
    }

    // tft.setTextColor(color, HX8357_BLACK);
    // // tft.setTextSize(2);
    // tft.setCursor(DISPLAY_XMIT_RECV_INDICATOR_X, DISPLAY_XMIT_RECV_INDICATOR_Y);
    strlpad(paddedString, string, ' ', sizeof(paddedString));
    // tft.print(paddedString);
    ui.displayMode(String(paddedString), (AColor)color);
}  // setIndicatorIcon()

/**
 * @brief Poll for and process touch events
 */
// This is calibration data for the raw touch data to the screen coordinates
// using 510 Ohm resistors to reduce the driven voltage to Y+ and X-
#define TS_MINX 123
#define TS_MINY 104
#define TS_MAXX 1715
#define TS_MAXY 1130
#define MINPRESSURE 120
static unsigned long lastTime = millis();  // For simple touchscreen debouncing
void process_touch() {
    TSPoint pi, pw;
    uint16_t draw_x, draw_y, touch_x, touch_y;

    unsigned long thisTime = millis();  // Milliseconds

    // DPRINTF("thisTime=%lu, lastTime=%lu\n", thisTime, lastTime);

    if ((thisTime - lastTime) >= 200) {  // Try to avoid touch bounces
        pi = ts.getPoint();              // Read the screen

        if (pi.z > MINPRESSURE) {  // Has screen been touched?
            DTRACE();
            pw.x = map(pi.x, TS_MINX, TS_MAXX, 0, 480);
            pw.y = map(pi.y, TS_MINY, TS_MAXY, 0, 320);

            DPRINTF("pi.x=%d pi.y=%d pi.z=%d\n", pi.x, pi.y, pi.z);

            AWidget::processTouch(pw.x, pw.y);  // Notify widgets that something has been touched

            // checkButton();
            // check_FT8_Touch();
            // check_WF_Touch();
            lastTime = thisTime;
        }
    }
}

/**
 * @brief Callback function notified when this MenuButton is touched
 */
void MenuButton::touchButton(int buttonId) {
    DPRINTF("touchButton #%d\n", buttonId);

#ifndef UNIT_TEST
    // Dispatch touch event into the application program
    switch (buttonId) {
        // CQ button
        case 0:
            DPRINTF("CQ\n");
            // If state=false then begin calling CQ, else cancel existing CQ
            // seq.cqButtonEvent();
            break;

        // Abort button
        case 1:
            DPRINTF("Ab\n");
            break;

        // Tune button
        case 2:
            DPRINTF("Tu\n");
            seq.tuneButtonEvent();
            break;

        // Enable transmit button
        case 3:
            DPRINTF("Tx\n");
            break;

        // Ignore unknown button identifiers
        default:
            DPRINTF("Unknown button\n");
            break;
    }
#endif

}  // touchButton()