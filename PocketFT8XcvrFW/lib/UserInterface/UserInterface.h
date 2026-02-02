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
#include "hwdefs.h"           //Pocket FT8 pin assignments for Teensy 4.1 MCU

// Transmit/Receive/Pending indicator icon
typedef enum {
    INDICATOR_ICON_RECEIVE = 0,   // Receive
    INDICATOR_ICON_PENDING = 1,   // Xmit awaiting timeslot
    INDICATOR_ICON_TRANSMIT = 2,  // Transmitting
    INDICATOR_ICON_TUNING = 3,    // Tuning
    INDICATOR_ICON_INITZN = 4     // Initializing
} IndicatorIconType;

// Station QSO message
typedef enum {
    QSO_MSG_XMITPEND = 0,  // Outbound msg awaiting timeslot for transmission
    QSO_MSG_XMITING = 1,   // Outbound msg during transmission
    QSO_MSG_XMITD = 2,     // Outbound sent (transmission complete) message
    QSO_MSG_XMITRPT = 3,   // Outbound message repeated
    QSO_MSG_RECVD = 4,     // Received message
    QSO_MSG_RECVRPT = 5,   // Repeated received
    QSO_MSG_DEBUG = 6      // Indicator used only for debugging
} QSOMsgEvent;

// Define the Waterfall widget's boundary and extent
static const ACoord WaterfallX = 0;          // Upper-left corner of Waterfall
static const ACoord WaterfallY = 0;          // Upper-left corner of Waterfall
static const APixelPos WaterfallRows = 105;  // #Pixel rows inside Waterfall widget
static const APixelPos WaterfallCols = 353;  // #Pixel cols inside Waterfall widget

// Define the Decoded Messages widget's boundary and extent
static const ACoord DecodedMsgsX = 0;     // Upper-left corner
static const ACoord DecodedMsgsY = 114;   // Upper-left corner
static const ALength DecodedMsgsW = 260;  // Width
static const ALength DecodedMsgsH = 172;  // Height

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
static const ACoord AppMsgX = 262;   // Upper-left corner
static const ACoord AppMsgY = 228;   // Upper-left corner
static const ALength AppMsgW = 218;  // Width
static const ALength AppMsgH = 60;   // Height

// Define the Button widgets' boundaries and extents
static const ALength ButtonSpacing = 61;  // Button grid spacing
static const ALength ButtonWidth = 50;    // Width in pixels
static const ALength ButtonHeight = 30;   // Height in pixels
static const ACoord ButtonX = 1;          // Button row left-side inset
static const ACoord ButtonY = 290;        // All buttons in one row at screen bottom

class QSOMessagesItem;

class QSOMessages : public AScrollBox {
   public:
    QSOMessages(ACoord x, ACoord y, ALength w, ALength h, AColor c) : AScrollBox(x, y, w, h, c) {
        // for (int i = 0; i < maxItems; i++) items[i] = nullptr;
        // pLastMsgItem = NULL;
    }
    void onTouchItem(AScrollBoxItem* pItem) override;  // Application overrides onTouchItem() to receive notifications of touch events
    QSOMessagesItem* addStationMessageItem(QSOMessages* pStationMessages, Decode* msg, QSOMsgEvent msgType);
    QSOMessagesItem* addStationMessageItem(QSOMessages* pStationMessages, String str, QSOMsgEvent msgType);
    // QSOMessagesItem* pLastMsgItem;  // The previously displayed QSO message

   private:
};

static String emptyString = String("");

/**
 * @brief Our record of a QSO message sent to or received by our station
 *
 * @note We record a copy of the entire Decode message object as some may be later referenced
 */
class QSOMessagesItem : public AScrollBoxItem {
   public:
    QSOMessagesItem(Decode* pNewMsg, AColor fgColor, AColor bgColor, QSOMessages* pBox) : AScrollBoxItem(emptyString, fgColor, bgColor, pBox) {
        // Sanity checks
        if ((pNewMsg == NULL) || (pBox == NULL)) return;

        // printf("%s:%u\n", __FILE__, __LINE__);
        msg = *pNewMsg;                                      // Retain a copy of the received msg struct
        pStationMessages = static_cast<QSOMessages*>(pBox);  // Save pointer to the base class object
    }  // QSOMessagesItem()
    QSOMessages* pStationMessages;
    Decode msg;  // A copy of the decoded message struct for this item
};

// Define the Waterfall class to implement theWaterfall "singleton" (well...) displaying
// the history of received signal levels vs. frequency.  Touching theWaterfall display
// moves the transmitter's offset (i.e. AFSK tone) frequency.
class Waterfall : public APixelBox {
   public:
    // uint16_t cursor_line;  // Pixel location of cursor_line within Waterfall widget

    Waterfall() : APixelBox(WaterfallX, WaterfallY, WaterfallRows, WaterfallCols) {}
    void onTouchPixel(APixelPos x, APixelPos y) override;  // Application overrides onTouchPixel() to receive notifications of touch events
    void initCursorFrequency(void);                        // Initializes the cursor frequency
};

// Define functions visible to legacy code
void pollTouchscreen(void);

// The interactive box displaying all decoded messages
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

/**
 * @brief The UserInterface is more-or-less implemented as a Meyers singleton
 *
 * @note The single instance of the UserInterface object is constructed by the
 * static declaration of theInstance variable.  A reference to that instance
 * is available through getInstance().
 *
 */
class UserInterface {
   public:
    // Public static method provides access to the UserInterface singleton
    static UserInterface& getInstance() {
        static UserInterface theInstance;  // This is the one-and-only instance of the UserInterface
        return theInstance;                // Return a reference to the UserInterface instance
    }
    // Initialization methods
    void begin(void);

    // StationInfo methods
    void displayFrequency(void);
    void displayLocator(String grid, AColor fgColor);
    void displayDate(bool forceUpdate = false);
    void displayTime(void);
    void displayCallsign();
    void displayMode(String mode, AColor fg);
    void setXmitRecvIndicator(IndicatorIconType indicator);

    // Methods to manage the Waterfall's cursor frequency
    void initCursorFrequency(void);                 // Initialize the Waterfall's cursor frequency
    void setCursorLine(uint16_t cursor_frequency);  // Move the cursor line (in pixels) for specified cursor frequency

    void endQSO(void);  // Clean-up UI following a QSO

    // Decorator methods for UI widgets
    void drawWaterfallPixel(APixelPos x, APixelPos y, AColor color);                     // Draws a pixel in the waterfall graph
    void displayQSOMsg(String s);                                                        // Display a message String in the QSO widget
    QSOMessagesItem* addStationMessageItem(QSOMessages* pStationMessages, Decode* msg);  // Add a decoded message to QSO
    QSOMessagesItem* addStationMessageItem(QSOMessages* pStationMessages, String str);   // Add a string to QSO

    // The widgets for displaying station info, traffic and info about the rig.
    // These really should have been singletons.
    Waterfall* theWaterfall;         // This is the Pocket FT8 waterfall graph
    AListBox* stationInfo;           // Information about our station
    DecodedMsgsBox* allDecodedMsgs;  // All FT8 messages decoded in the previous timeslot
    QSOMessages* theQSOMsgs;         // FT8 messages transmitted to/from our station
    ATextBox* applicationMsgs;       // Status/error messages

    // Waterfall attributes
    uint16_t cursor_line;  // X-Offset in pixels of the cursor within the Waterfall widget

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

    // The UserInterface's statics
    static int lastDay;  // Helps determine when date has changed

   private:
    UserInterface() {}                                        // Singleton's inaccessible constructor
    UserInterface(const UserInterface&) = delete;             // Delete singleton's copy constructor
    UserInterface& operator=(const UserInterface&) = delete;  // Delete assignment operator
};
