#pragma once

#include <Arduino.h>  //We use many Arduino classes and data types

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

// Define the Waterfall widget's boundary and extent
static const ACoord WaterfallX = 0;          // Upper-left corner of Waterfall
static const ACoord WaterfallY = 0;          // Upper-left corner of Waterfall
static const APixelPos WaterfallRows = 100;  // #Pixel rows inside Waterfall widget
static const APixelPos WaterfallCols = 353;  // #Pixel cols inside Waterfall widget

// Define the UTC Date widget's boundary and extent
static const ACoord DateX = 360;   // Upper-left corner of UTC Date
static const ACoord DateY = 0;     // Upper-left corner of UTC Date
static const ALength DateW = 120;  // Width
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

    // Declare the Pocket FT8 widgets
    Waterfall* waterfallBox;
    // ATextBox dateBox = ATextBox("04/11/2025", DateX, DateY, DateW, 40);
    // ATextBox timeBox = ATextBox("20:49:58", TimeX, TimeY, TimeW, TimeH);
    AListBox* infoBox;

   private:
};
