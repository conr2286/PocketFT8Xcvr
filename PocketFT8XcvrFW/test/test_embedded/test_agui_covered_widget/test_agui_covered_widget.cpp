/**
 * This test exercises how AGUI restores a widget's displayed image when
 * another widget, whose image covered the first at least in part, is destroyed.
 *
 */

#include <Arduino.h>       //
#include <Adafruit_GFX.h>  //Must include this header prior to HX8357_t3n.h

// Underlying access to HW touchscreen, display and fonts
#include "pins.h"             //Pocket FT8 pin assignments for Teensy 4.1 MCU
#include "HX8357_t3n.h"       //HX8357 specific defn's
#include "TouchScreen_I2C.h"  //MCP342X interface to Adafruit's 2050 touchscreen
#include "FT8Font.h"          //Customized font for the Pocket FT8 Revisited

// AGUI definitions
#include "AColor.h"         //AGUI colors
#include "ACoord.h"         //Screen coordinate data types
#include "AGUI.h"           //The Graphical User Interface defn's
#include "AListBox.h"       //Interactive text box
#include "APixelBox.h"      //Interactive raster box
#include "AScrollBox.h"     //Scrolling interactive text box
#include "ATextBox.h"       //Non-interactive text
#include "AToggleButton.h"  //Stateful button

// Testing and debugging
#include "unity.h"
#include "DEBUG.h"

// This is calibration data for the raw touch data to the screen coordinates
// using 510 Ohm resistors to reduce the driven voltage to Y+ and X-
#define TS_MINX 123
#define TS_MINY 104
#define TS_MAXX 1715
#define TS_MAXY 1130
#define MINPRESSURE 120  // I don't think the pressure stuff is working as expected

// Build the underlying TFT display, touchscreen and AGUI objects
HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_RST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins
TouchScreen ts = TouchScreen(PIN_XP, PIN_YP, PIN_XM, PIN_YM, 282);                   // The 282 ohms is the measured x-Axis resistance of 3.5" Adafruit touchscreen in 2024
static AGUI gui(&tft, 3, &FT8Font);

// Define the test widgets
AScrollBox w0(0, 0, 480, 320, A_DARK_GREY);  // The lowest level widget, lying beneath the others
AListBox w1(20, 20, 120, 120, A_BLUE);       // Widget w1 sits left of w2, partly covering w0
APixelBox w2(200, 20, 120, 120);             // Widget w2 sits right of w1, partly covering w0
AScrollBox w3(60, 60, 240, 40, A_WHITE);     // Widget w3 partially covers both w1 and w2

// Define the ContinueButton, overriding AToggleButton::onTouchButton to receive touch notifications
class ContinueButton : public AToggleButton {
   public:
    ContinueButton() : AToggleButton("CONTINUE", 40, 240, 100, 40, 0, true) {}
    void onTouchButton(int userData) override { return; };  // We override AToggleButton to receive notifications of touch events
} continueButton;

// Define the AbortButton, overriding AToggleButton::onTouchButton to receive touch notifications.
// Note:  Clicking the ABORT button terminates the test.
class AbortButton : public AToggleButton {
   public:
    AbortButton() : AToggleButton("ABORT", 200, 240, 100, 40, 0, true) {};
    void onTouchButton(int userData) override {
        UNITY_END();
    }  // onTouchButton()
} abortButton;

/**
 * @brief Poll for and process touch events
 *
 * @note This function is invoked only by loop() to poll for touch screen events
 *
 * To minimize bounce and delays arising from ADC conversions that are not
 * really required, we pace the touchscreen (i.e. getPoint()) measurements.
 */
unsigned long lastTime = millis();  // Time when we last polled the touchscreen
void pollTouchscreen() {
    TSPoint pi, pw;
    unsigned long thisTime = millis();  // Current time

    if ((thisTime - lastTime) >= 250) {  // 250 mS between measurements
        pi = ts.getPoint();              // Read the screen

        if (pi.z > MINPRESSURE) {                        // Has screen been touched?
            pw.x = map(pi.x, TS_MINX, TS_MAXX, 0, 480);  // Map to resistance to screen coordinate
            pw.y = map(pi.y, TS_MINY, TS_MAXY, 0, 320);

            // AWidget can determine which widget was touched and notify it
            AWidget::processTouch(pw.x, pw.y);  // Notify all widgets that something was touched
            lastTime = thisTime;                // Note the time when we last took a measurement
        }
    }
}  // pollTouchScreen()

//
void test_StationInfo(void) {
    TEST_MESSAGE("test_StationInfo()\n");
}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_StationInfo);
    return UNITY_END();
}

void setup() {
    Serial.begin(9600);

    runUnityTests();
    delay(1000);
}

void tearDown(void) {
}

// loop() polls touchscreen for button presses.  The button widget objects react to presses.
void loop() {
    pollTouchscreen();
}
