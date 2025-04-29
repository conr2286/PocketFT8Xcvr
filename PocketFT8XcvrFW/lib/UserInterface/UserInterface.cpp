/**
 * @brief The Pocket FT8 Revisited (PFR) User Interface
 *
 * @note UserInterface.cpp attempts to consolodate all user interface decisions and objects
 * in this single location facilitating future tweeks to the hardware and design.
 *
 * #ifndef PIO_UNIT_TESTING is widely used to disable application code to allow the
 * user interface to be unit tested by itself.
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
#include "Config.h"     // CONFIG.JSON
#include "FT8Font.h"     //Customized font for the Pocket FT8 Revisited
#include "GPShelper.h"   //Decorator for Adafruit_GPS library
#include "HX8357_t3n.h"  //WARNING:  #include HX8357_t3n following Adafruit_GFX
#include "NODEBUG.h"     //USB Serial debugging on the Teensy 4.1
#include "PocketFT8Xcvr.h"  //Globals
#include "Sequencer.h"  //RoboOp
#include "TouchScreen_I2C.h"  //MCP342X interface to Adafruit's 2050 touchscreen
#include "decode_ft8.h"       //Decoded message types
#include "lexical.h"  //String helpers
#include "pins.h"     //Pocket FT8 pin assignments for Teensy 4.1 MCU
#include "traffic_manager.h"

HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_RST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins
TouchScreen ts = TouchScreen(PIN_XP, PIN_YP, PIN_XM, PIN_YM, 282);                   // The 282 ohms is the measured x-Axis resistance of 3.5" Adafruit touchscreen in 2024
static AGUI* gui;

// Define externals required to build the application (as opposed to unit test)
#ifndef PIO_UNIT_TESTING
static Sequencer& seq = Sequencer::getSequencer();  // Get a reference to the Sequencer (RoboOp)
#endif

extern Config config;

// GPS Access
extern GPShelper gpsHelper;  // TODO:  This shouldn't be an extern :()
extern UserInterface ui;

void DecodedMsgsBox::setMsg(int index, char* msg) {
    setItem(index, msg, A_WHITE, bgColor);
}


// /**
//  * Helper function to pad a char[] to a specified length
//  *
//  * @param str  Pointer to the char[] string to pad
//  * @param size sizeof(str) including the NUL terminator
//  * @param c    The pad character (e.g. ' ')
//  *
//  * strlpad() pads the specified array containing a NUL terminated char string
//  * with the specified char, c, ensuring that str remains NUL terminated.  The
//  * resulting string, including the NUL terminator, will occupy no more than
//  * size chars in str[].
//  *
//  **/
// static char* strlpad(char* str, int size, char c) {
//     const char NUL = 0;
//     bool paddingUnderway = false;
//     int i;

//     for (i = 0; i < size - 1; i++) {
//         if (str[i] == NUL) paddingUnderway = true;
//         if (paddingUnderway) {
//             str[i] = c;
//         }
//     }

//     str[size - 1] = NUL;

//     return str;

// }  // strlpad()





/**
 * @brief Start-up the Adafruit Display, GFX adapter and library, the resistive touchscreen, and widgets
 */
void UserInterface::begin() {
    // DTRACE();

    // Define the interfaces/adapters for accessing the underlying graphics libraries and hardware
    gui = new AGUI(&tft, 3, &FT8Font);  // Graphics adapter insulation from the multitude of Adafruit GFX libraries
    // ts = new TouchScreen(PIN_XP, PIN_YP, PIN_XM, PIN_YM, 282);                    // 282 ohms is the measured x-Axis resistance of my Adafruit 2050 touchscreen

    // Build the Waterfall
    theWaterfall = new Waterfall();

    // Build the stationInfo
    // stationInfo = new AScrollBox(InfoX, InfoY, InfoW, InfoH, A_DARK_GREY);
    stationInfo = new AListBox(InfoX, InfoY, InfoW, InfoH, A_DARK_GREY);
    itemDate = stationInfo->addItem(stationInfo, "", A_RED);
    itemTime = stationInfo->addItem(stationInfo, "", A_RED);
    itemLocator = stationInfo->addItem(stationInfo, "", A_RED);
    itemCallsign = stationInfo->addItem(stationInfo, "");
    itemFrequency = stationInfo->addItem(stationInfo, "");
    itemMode = stationInfo->addItem(stationInfo, "");

    // Build the decoded messages box
    decodedMsgs = new DecodedMsgsBox(DecodedMsgsX, DecodedMsgsY, DecodedMsgsW, DecodedMsgsH, A_DARK_GREY);

    // Build the station messages box
    stationMsgs = new StationMessages(StationMsgsX, StationMsgsY, StationMsgsW, StationMsgsH, A_DARK_GREY);

    // // Application message box
    applicationMsgs = new ATextBox("", AppMsgX, AppMsgY, AppMsgW, AppMsgH, A_DARK_GREY);

    // Build the buttons
    b0 = new MenuButton("CQ", ButtonX + 0 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 0);
    b1 = new MenuButton("Ab", ButtonX + 1 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 1);
    b2 = new MenuButton("Tu", ButtonX + 2 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 2);
    b3 = new MenuButton("Tx", ButtonX + 3 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 3);
    b4 = new MenuButton("M0", ButtonX + 4 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 4);
    b5 = new MenuButton("M1", ButtonX + 5 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 5);
    b6 = new MenuButton("M2", ButtonX + 6 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 6);
    b7 = new MenuButton("SY", ButtonX + 7 * ButtonSpacing, ButtonY, ButtonWidth, ButtonHeight, 7);
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
        // Do we have disciplined UTC date from GPS?
        if (gpsHelper.validGPSdata) {
            fg = A_GREEN;
        } else {
            fg = A_YELLOW;
        }
        itemDate->setItemText(String(str), fg);  // Green if GPS Disciplined
        lastDay = thisDay;                       // Remember new date
    }
}  // displayDate()

/**
 * @brief Display UTC time
 *
 * Displayed in green if GPS disciplined else yellow.
 *
 * We only update the display infrequently to mitigate flicker
 */
static int lastSecond = -1;
void UserInterface::displayTime() {
    Teensy3Clock.get();  // Sync MCU clock with RTC
    char str[13];        // print format stuff
    AColor fg;
    int thisSecond = second();
    if (abs(thisSecond - lastSecond) >= 1) {
        snprintf(str, sizeof(str), "%02i:%02i:%02i", hour(), minute(), second());
        if (gpsHelper.validGPSdata) {
            fg = A_GREEN;
        } else {
            fg = A_YELLOW;
        }
        itemTime->setItemText(String(str), fg);  // Green if GPS Disciplined
        lastSecond = thisSecond;
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
    String str;

    switch (indicator) {
        // We are receiving
        case INDICATOR_ICON_RECEIVE:
            color = HX8357_GREEN;
            str = "RECEIVE";
            break;
        // Transmission pending for next appropriate timeslot
        case INDICATOR_ICON_PENDING:
            color = HX8357_YELLOW;
            str = "PENDING";
            break;
        // Transmission in progress
        case INDICATOR_ICON_TRANSMIT:
            color = HX8357_RED;
            str = "TRANSMIT";
            break;
        // Tuning in progress
        case INDICATOR_ICON_TUNING:
            color = HX8357_ORANGE;
            str = "TUNING";
            break;
        // Lost in the ozone again
        default:
            color = HX8357_BLACK;
            str = " ";
            break;
    }

    ui.displayMode(str, (AColor)color);
}  // setIndicatorIcon()

// This is calibration data for the raw touch data to the screen coordinates
// using 510 Ohm resistors to reduce the driven voltage to Y+ and X-
#define TS_MINX 123
#define TS_MINY 104
#define TS_MAXX 1715
#define TS_MAXY 1130
#define MINPRESSURE 120  // I don't think the pressure stuff is working as expected

// Remember the time when we previously checked the touchscreen
static unsigned long lastTime = millis();

/**
 * @brief Poll for and process touch events
 *
 * @note This function is invoked only by loop() to poll for touch screen events
 *
 * To minimize bounce and delays arising from ADC conversions that are not
 * really required, we pace the touchscreen (i.e. getPoint()) measurements.
 */
void pollTouchscreen() {
    TSPoint pi, pw;
    // uint16_t draw_x, draw_y, touch_x, touch_y;

    unsigned long thisTime = millis();  // Current time

    if ((thisTime - lastTime) >= 250) {  // 250 mS between measurements
        pi = ts.getPoint();              // Read the screen

        if (pi.z > MINPRESSURE) {                        // Has screen been touched?
            pw.x = map(pi.x, TS_MINX, TS_MAXX, 0, 480);  // Map to resistance to screen coordinate
            pw.y = map(pi.y, TS_MINY, TS_MAXY, 0, 320);

            // AWidget can determine which widget was touched and notify it
            AWidget::processTouch(pw.x, pw.y);  // Notify widgets that something was touched

            // check_FT8_Touch();
            // check_WF_Touch();
            lastTime = thisTime;  // Note the time when we last took a measurement
        }
    }
}

/**
 * @brief Callback function notified when this MenuButton is touched
 */
void MenuButton::onTouchButton(int buttonId) {
    DPRINTF("onTouchButton #%d\n", buttonId);

#ifndef PIO_UNIT_TESTING              // Omit application product code when compiling tests
    ui.applicationMsgs->setText("");  // Clear existing application message, if any

    // Dispatch touch event into the application program
    switch (buttonId) {
        // CQ button
        case 0:
            DPRINTF("CQ\n");
            // If state=false then begin calling CQ, else cancel existing CQ
            seq.cqButtonEvent();
            break;

        // Abort button
        case 1:
            DPRINTF("Ab\n");
            seq.abortButtonEvent();       // Ask Sequencer to abort transmissions
            ui.b1->setState(false);       // Turn button "off" (it doesn't really toggle)
            ui.b1->onRepaintWidget();     // Repaint the now "off" button
            ui.applicationMsgs->reset();  // Reset (clear) the Application Messages box
            break;

        // Tune button
        case 2:
            DPRINTF("Tu\n");
            seq.tuneButtonEvent();
            break;

        // Control RoboOp to automagically reply to a received CQ message
        case 3:
            DPRINTF("Tx\n");
            if (getState()) {
                setAutoReplyToCQ(true);  // If button is on then enable RoboOp
                if (config.enableDuplicates) {
                    ui.applicationMsgs->setText("RoboOp is enabled and will automatically\nreply to CQs from any station, even dups");
                } else {
                    ui.applicationMsgs->setText("RoboOp is enabled and will automatically\nreply to CQ from an unlogged station");
                }
            } else {
                setAutoReplyToCQ(false);  // Else disable RoboOp
                ui.applicationMsgs->setText("RoboOp is disabled");
            }
            break;

        // Ignore unknown button identifiers
        default:
            DPRINTF("Unknown button\n");
            break;
    }
#endif
}  // onTouchButton()

/**
 * @brief Override APixelBox to receive notifications of touch events
 * @param x X-Coord
 * @param y Y-Coord
 *
 * @note The coordinates received from APixelBox are inside the bitmap, not
 * screen coordinates.
 */
extern uint16_t cursor_freq;  // Frequency of cursor line
extern uint16_t cursor_line;  // Pixel location of cursor line
#define ft8_min_bin 48
#define FFT_Resolution 6.25
const float ft8_shift = 6.25;  // FT8 Hz/bin???

void Waterfall::onTouchPixel(ACoord x, ACoord y) {
#ifndef PIO_UNIT_TESTING
    cursor_line = x;
    cursor_freq = ((float)cursor_line + (float)ft8_min_bin) * ft8_shift;
    set_Xmit_Freq();
    String str = String("Cursor freq = ") + String(cursor_freq) + String(" Hz");
    // DPRINTF("%s\n", str.c_str());
    ui.applicationMsgs->setText(str);
#endif

}  // onTouchPixel()

/**
 * @brief Override AListBox to receive touch notifications for list items
 * @param index Index of touched item
 * @param isSelected true if item is selected
 *
 * When operator touches an item in DecodedMsgsBox, the Sequencer will attempt to
 * engage that station in a QSO.
 */
void DecodedMsgsBox::onTouchItem(AListBoxItem* pItem) {
    DPRINTF("onTouchItem(index=%d,)\n", index);
    pItem->setItemColors(A_BLACK, A_GREY);  // Highlight this item
#ifndef PIO_UNIT_TESTING
    int index = getItemIndex(pItem);      // Lookup item's index
    seq.decodedMessageClickEvent(index);  // Notify Sequencer when operator clicks a received message
#endif
}
/**
 * @brief Tokenizer for FT8 messages
 * @param str Reference to the String being tokenized
 * @return First token from str
 *
 * @brief Extracts the first token (String of non-space chars) from front of str
 *
 * @note Modifies str text by removing the first token from the referenced str
 *
 */
static String getNextStringToken(String& str) {
    String sp(" ");

    // Skip leading spaces and ensure a trailing space
    str = str.trim() + sp;
    // DPRINTF("Trimmed incoming str='%s'\n", str.c_str());

    // Find the space terminating the token
    int index = str.indexOf(' ');  // Find the space terminating the token
    // DPRINTF("index=%d\n", index);
    if (index <= 0) return emptyString;      // Missing?
    String token = str.substring(0, index);  // Extract token
    str = str.substring(index);              // Trim token from str
    // DPRINTF("Token='%s', remainder str='%s'\n", token.c_str(),str.c_str());
    return token;
}  // getNextStringToken()

/**
 * @brief Override notified when StationMessage item touched
 * @param pItem
 */
void StationMessages::onTouchItem(AScrollBoxItem* pItem) {
    DPRINTF("touched text %s\n", pItem->getItemText()->c_str());

    StationMessagesItem* pMsgItem = static_cast<StationMessagesItem*>(pItem);

    DPRINTF("field1=%s field2=%s field3=%s\n", pMsgItem->msg.field1, pMsgItem->msg.field2, pMsgItem->msg.field3);

    // Ignore touch on our own transmitted message
#ifndef PIO_UNIT_TESTING
    if ((strcmp(pMsgItem->msg.field2, thisStation.getCallsign()) == 0)) return;
#endif

    // Highlight the touched Station Message
    pMsgItem->setItemColors(A_BLACK, A_LIGHT_GREY);

    // Ask the Sequencer (RoboOp) to contact the remote station identified in the touched message
    DPRINTF("Contact %s\n", pMsgItem->msg.field2);
#ifndef PIO_UNIT_TESTING
    seq.decodedMessageClickEvent(&(pMsgItem->msg));
#endif

}  // StationMessages::onTouchItem()

StationMessagesItem* StationMessages::addStationMessageItem(StationMessages* pContainer, String str) {
    Decode newMsg;

    // Extract fields from str
    // DPRINTF("%s %s %s\n", getNextStringToken(str).c_str(), getNextStringToken(str).c_str(), getNextStringToken(str).c_str());
    strcpy(newMsg.field1, getNextStringToken(str).c_str());
    strcpy(newMsg.field2, getNextStringToken(str).c_str());
    strcpy(newMsg.field3, getNextStringToken(str).c_str());

    DPRINTF("%s %s %s \n", newMsg.field1, newMsg.field2, newMsg.field3);

    StationMessagesItem* newItem = addStationMessageItem(pContainer, &newMsg);
    DTRACE();

    return newItem;
}

/**
 * @brief Add an item to this Station Messages Box
 * @param pStationMessages Back pointer to Station Messages Box
 * @param pNewMsg New Decode msg structure
 * @return Pointer to new item or nullptr
 */
StationMessagesItem* StationMessages::addStationMessageItem(StationMessages* pStationMessages, Decode* pNewMsg) {
    int newItemIndex = nDisplayedItems;
    StationMessagesItem* pNewItem;
    String str = pNewMsg->toString();

    DPRINTF("field1=%s field2=%s field3=%s\n", pNewMsg->field1, pNewMsg->field2, pNewMsg->field3);

    // Too many items for our simple data structs?
    if (nDisplayedItems >= maxItems) return nullptr;

    pNewItem = new StationMessagesItem(pNewMsg, A_WHITE, A_BLACK, pStationMessages);  // Derived of AScrollBoxItem
    if (pNewItem != nullptr) {
        pNewItem->setItemText(pNewMsg->toString());
        items[newItemIndex] = pNewItem;
    }

    // Record the timestamp
    pNewItem->timeStamp = millis();  // Record timestamp when item created

    // Scroll the displayed items up if the added item won't fit within widget's boundary box
    if (!itemWillFit(nDisplayedItems + 1)) {
        DPRINTF("Scroll with nDisplayedItems=%d\n", nDisplayedItems);
        scrollUpOneLine();  // Scrolling reduces nDisplayedItems by one, making room for new item
    }

    // Record the new item
    nDisplayedItems++;  // Bump count of displayed items
    displayedItems[newItemIndex] = pNewItem;

    // Paint the new item
    repaint(pNewItem);

    return pNewItem;
}
