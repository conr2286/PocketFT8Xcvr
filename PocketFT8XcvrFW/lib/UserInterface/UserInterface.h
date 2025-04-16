#pragma once

#include <Arduino.h>  //We use many Arduino classes and data types

#include "AColor.h"           //AGUI colors
#include "ACoord.h"           // Screen coordinate data types
#include "AGUI.h"             //The adapter for Adafruit GFX libraries
#include "AListBox.h"         //Interactive text box
#include "AScrollBox.h"       //Scrolling interactive text box
#include "APixelBox.h"        //Interactive raster box
#include "ATextBox.h"         //Non-interactive text
#include "AToggleButton.h"    //Stateful button
#include "DEBUG.h"            //USB Serial debugging on the Teensy 4.1
#include "FT8Font.h"          //Customized font for the Pocket FT8 Revisited
#include "HX8357_t3n.h"       //WARNING:  #include HX8357_t3n following Adafruit_GFX
#include "TouchScreen_I2C.h"  //MCP342X interface to Adafruit's 2050 touchscreen
#include "pins.h"             //Pocket FT8 pin assignments for Teensy 4.1 MCU

// Define the Waterfall widget's boundary and extent
static const ACoord WaterfallX = 0;          // Upper-left corner of Waterfall
static const ACoord WaterfallY = 0;          // Upper-left corner of Waterfall
static const APixelPos WaterfallRows = 100;  // #Pixel rows inside Waterfall widget
static const APixelPos WaterfallCols = 353;  // #Pixel cols inside Waterfall widget

// Define the Decoded Messages widget's boundary and extent
static const ACoord DecodedMsgsX = 0;     // Upper-left corner
static const ACoord DecodedMsgsY = 108;   // Upper-left corner
static const ALength DecodedMsgsW = 260;  // Width
static const ALength DecodedMsgsH = 110;  // Height

// Define the Station Messages widget's boundary and extent
static const ACoord StationMsgsX = 262;   // Upper-left corner
static const ACoord StationMsgsY = 108;   // Upper-left corner
static const ALength StationMsgsW = 218;  // Width
static const ALength StationMsgsH = 110;  // Height

// Define the UTC Date widget's boundary and extent
static const ACoord DateX = 361;   // Upper-left corner of UTC Date
static const ACoord DateY = 0;     // Upper-left corner of UTC Date
static const ALength DateW = 119;  // Width
static const ALength DateH = 20;   // Height

// Define the UTC Time widget's boundary and extent
static const ACoord TimeX = 360;   // Upper-left corner of UTC Time
static const ACoord TimeY = 20;    // Upper-left corner of UTC Time
static const ALength TimeW = 120;  // Width
static const ALength TimeH = 20;   // Height

// Define the Station widget's boundary and extent
static const ACoord StationX = 360;   // Upper-left corner of Station
static const ACoord StationY = 40;    // Upper-left corner of Station
static const ALength StationW = 120;  // Width
static const ALength StationH = 20;   // Height

//Define Application Message boundary and extent
static const ACoord AppMsgX = 0;  //Upper-left corner
static const ACoord AppMsgY = 220; //Upper-left corner
static const ALength AppMsgW = 480; //Width
static const ALength AppMsgH = 40; //Height

// Define the Button widgets' boundaries and extents
static const ALength ButtonSpacing = 60;  // Button grid spacing
static const ALength ButtonWidth = 50;    // Width in pixels
static const ALength ButtonHeight = 30;   // Height in pixels
static const ACoord ButtonY = 290;        // All buttons in one row at screen bottom

class Waterfall : public APixelBox {
   public:
    Waterfall() : APixelBox(WaterfallX, WaterfallY, WaterfallRows, WaterfallCols) {}
};

class UserInterface {
   private:
    // Define the interfaces/adapters for accessing the underlying graphics libraries and hardware
    HX8357_t3n* tft;
    AGUI* gui;
    TouchScreen* ts;

   public:
    void begin(void);

    // The widgets for displaying station info, traffic and info about the rig
    Waterfall* theWaterfall;
    AListBox* stationInfo;
    AScrollBox* decodedMsgs;
    AScrollBox* stationMsgs;
    ATextBox* appMessage;

    // The button widgets
    AToggleButton* b0;
    AToggleButton* b1;
    AToggleButton* b2;
    AToggleButton* b3;
    AToggleButton* b4;
    AToggleButton* b5;
    AToggleButton* b6;
    AToggleButton* b7;

   private:
};
