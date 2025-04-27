#pragma once

#include <Arduino.h>  //We use many Arduino classes and data types

#include "AColor.h"           //AGUI colors
#include "ACoord.h"           // Screen coordinate data types
#include "AGUI.h"             //The adapter for Adafruit GFX libraries
#include "AListBox.h"         //Interactive text box
#include "APixelBox.h"        //Interactive raster box
#include "AScrollBox.h"       //Scrolling interactive text box
#include "ATextBox.h"         //Non-interactive text
#include "AToggleButton.h"    //Stateful button
#include "FT8Font.h"          //Customized font for the Pocket FT8 Revisited
#include "HX8357_t3n.h"       //WARNING:  #include HX8357_t3n following Adafruit_GFX
#include "TouchScreen_I2C.h"  //MCP342X interface to Adafruit's 2050 touchscreen
#include "decode_ft8.h"       //Decoded message types
#include "pins.h"             //Pocket FT8 pin assignments for Teensy 4.1 MCU

// Transmit/Receive/Pending indicator icon
typedef enum {
    INDICATOR_ICON_RECEIVE = 0,
    INDICATOR_ICON_PENDING = 1,
    INDICATOR_ICON_TRANSMIT = 2,
    INDICATOR_ICON_TUNING = 3
} IndicatorIconType;

// Define the Waterfall widget's boundary and extent
static const ACoord WaterfallX = 0;          // Upper-left corner of Waterfall
static const ACoord WaterfallY = 0;          // Upper-left corner of Waterfall
static const APixelPos WaterfallRows = 105;  // #Pixel rows inside Waterfall widget
static const APixelPos WaterfallCols = 353;  // #Pixel cols inside Waterfall widget

// Define the Decoded Messages widget's boundary and extent
static const ACoord DecodedMsgsX = 0;     // Upper-left corner
static const ACoord DecodedMsgsY = 114;   // Upper-left corner
static const ALength DecodedMsgsW = 260;  // Width
static const ALength DecodedMsgsH = 112;  // Height

// Define our Station Messages widget's boundary and extent
static const ACoord StationMsgsX = 262;   // Upper-left corner
static const ACoord StationMsgsY = 114;   // Upper-left corner
static const ALength StationMsgsW = 218;  // Width
static const ALength StationMsgsH = 112;  // Height

// Define the Info widget's boundary and extent
static const ACoord InfoX = 361;   // Upper-left corner of UTC Date
static const ACoord InfoY = 0;     // Upper-left corner of UTC Date
static const ALength InfoW = 119;  // Width
static const ALength InfoH = 112;  // Height

// Define Application Message boundary and extent
static const ACoord AppMsgX = 0;     // Upper-left corner
static const ACoord AppMsgY = 228;   // Upper-left corner
static const ALength AppMsgW = 480;  // Width
static const ALength AppMsgH = 60;   // Height

// Define the Button widgets' boundaries and extents
static const ALength ButtonSpacing = 61;  // Button grid spacing
static const ALength ButtonWidth = 50;    // Width in pixels
static const ALength ButtonHeight = 30;   // Height in pixels
static const ACoord ButtonX = 1;          // Button row left-side inset
static const ACoord ButtonY = 290;        // All buttons in one row at screen bottom

class StationMessagesItem;

class StationMessages : public AScrollBox {
   public:
    StationMessages(ACoord x, ACoord y, ALength w, ALength h, AColor c) : AScrollBox(x, y, w, h, c) {
        for (int i = 0; i < maxItems; i++) items[i] = nullptr;
    }
    void onTouchItem(AScrollBoxItem* pItem) override;  // Application overrides onTouchItem() to receive notifications of touch events
    StationMessagesItem* addStationMessageItem(StationMessages* pStationMessages, Decode* msg);
    StationMessagesItem* addStationMessageItem(StationMessages* pStationMessages, String str);

   private:
    StationMessagesItem* items[maxItems];
};

static String emptyString = String("");

class StationMessagesItem : public AScrollBoxItem {
   public:
    StationMessagesItem(Decode* pNewMsg, AColor fgColor, AColor bgColor, StationMessages* pBox) : AScrollBoxItem(emptyString, fgColor, bgColor, pBox) {
        msg = *pNewMsg;                                          // Retain a copy of the received msg struct
        pStationMessages = static_cast<StationMessages*>(pBox);  // Save pointer to the base class object
    }  // StationMessagesItem()
    StationMessages* pStationMessages;
    Decode msg;  // The decoded message struct for this item
};

class Waterfall : public APixelBox {
   public:
    Waterfall() : APixelBox(WaterfallX, WaterfallY, WaterfallRows, WaterfallCols) {}
    void onTouchPixel(APixelPos x, APixelPos y) override;  // Application overrides onTouchPixel() to receive notifications of touch events
};

// Define functions visible to legacy code
void pollTouchscreen(void);

// The interactive box displaying decoded messages
class DecodedMsgsBox : public AListBox {
   public:
    DecodedMsgsBox(ACoord x, ACoord y, ALength w, ALength h, AColor c) : AListBox(x, y, w, h, c) {}
    void onTouchItem(AListBoxItem* pItem) override;
    void setMsg(int index, char* msg);
};

// The application's menu buttons
class MenuButton : public AToggleButton {
   public:
    MenuButton(const char* str, ACoord x1, ACoord y1, ALength w, ALength h, int userData) : AToggleButton(str, x1, y1, w, h, userData, true) {}
    void onTouchButton(int userData) override;  // We override AToggleButton to receive notifications of touch events
};

class UserInterface {
   public:
    // Initialization methods
    void begin(void);

    // StationInfo methods
    void displayFrequency(unsigned kHz);
    void displayLocator(String grid, AColor fgColor);
    void displayDate(bool forceUpdate = false);
    void displayTime(void);
    void displayCallsign(String callSign);
    void displayMode(String mode, AColor fg);
    void setXmitRecvIndicator(IndicatorIconType indicator);

    // The widgets for displaying station info, traffic and info about the rig
    Waterfall* theWaterfall;
    AListBox* stationInfo;
    DecodedMsgsBox* decodedMsgs;
    StationMessages* stationMsgs;
    ATextBox* applicationMsgs;

    // The stationInfo items
    AListBoxItem* itemDate;
    AListBoxItem* itemTime;
    AListBoxItem* itemLocator;
    AListBoxItem* itemCallsign;
    AListBoxItem* itemFrequency;
    AListBoxItem* itemMode;

    // The button widgets
    MenuButton* b0;
    MenuButton* b1;
    MenuButton* b2;
    MenuButton* b3;
    MenuButton* b4;
    MenuButton* b5;
    MenuButton* b6;
    MenuButton* b7;
};
