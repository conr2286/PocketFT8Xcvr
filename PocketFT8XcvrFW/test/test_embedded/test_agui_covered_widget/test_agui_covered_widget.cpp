/**
 * This test suite exercises how AGUI restores a widget's displayed image when
 * another widget, whose image covered the first at least in part, is destroyed.
 *
 * The suite constructs a stack of five widgets as follows:
 *  w0 Lies beneath (at the bottom of) all other widgets
 *  w1 Placed above w0 and to the left of w2
 *  w2 Placed above w0 and to the right of w1
 *  w3 Partially covers w0, w1 and w2
 *  w4 This widget is not really part of the test, it's used for displaying messages to the operator
 * There are also two buttons, PASS and FAIL.
 *
 * The test suite begins by displaying the widgets, a message for the operator, and awaiting permission
 * to PASS with the first test case.
 *
 * Each test case modifies the displayed widgets, displays a message naming the executing test,
 * and requesting the operator's permission to either PASS the test suite or FAIL the
 * remaining tests.  If the displayed widgets are incorrect (see notes for each test case) then
 * the operator should FAIL the tests suite.
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
AScrollBox* w0 = new AScrollBox(0, 0, 480, 160, A_DARK_GREY);  // The lowest level widget, lying beneath all others
AListBox* w1 = new AListBox(20, 20, 120, 120, A_BLUE);         // Widget w1 sits left of w2, partly covering w0
AListBox* w2 = new AListBox(200, 20, 120, 120, A_GREEN);       // Widget w2 sits right of w1, partly covering w0
ATextBox* w3 = new ATextBox("w3", 60, 60, 220, 40, A_WHITE);   // Widget w3 partially covers both w1, w2 and w0
ATextBox* w4 = new ATextBox("", 0, 170, 300, 60, A_GREY);

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

// Define the PASS button by overriding AToggleButton::onTouchButton to receive touch notifications
class PassButton : public AToggleButton {
    bool waitForButton = true;

   public:
    PassButton() : AToggleButton("PASS", 40, 240, 100, 40, 0, true) {}
    void onTouchButton(int userData) override { waitForButton = false; };  // We override AToggleButton to receive notifications of touch events
    // Loop, waiting for a touchscreen press
    void wait(void) {
        while (waitForButton) {
            pollTouchscreen();  // Notifies any widget (including PASS and FAIL) of a click within
            delay(50);
        }
        waitForButton = true;  // Reset
    }  // wait()
} PassButton;

// Define the FailButton, overriding AToggleButton::onTouchButton to receive touch notifications.
// Note:  Clicking the Fail button terminates the test.
// Note:  We only expect FailButton to be pressed while we are waiting on Pass
class FailButton : public AToggleButton {
   public:
    FailButton() : AToggleButton("FAIL", 200, 240, 100, 40, 0, true) {};
    void onTouchButton(int userData) override {
        TEST_FAIL();
        UNITY_END();
    }  // onTouchButton()
} FailButton;

/**
 * @brief Display name of executing test and ask operator if it PASSed or FAILed
 *
 * @note We display the test name in w4 and then loop, waiting for the PASS (or FAIL) button press
 */
void passOrFail(String msg) {
    w4->setText(String("\n  Testing ") + msg);
    PassButton.wait();
}

/**
 * @brief Remove widget w2 from the middle of the stack
 *
 * @note When this test pauses, widget w2 should be removed from the display.
 * All other widgets, present prior to the execution of remove_w2, should remain.
 */
void remove_w2(void) {
    delete w2;
    passOrFail(__FUNCTION__);
}

/**
 * @brief Remove widget w4 from the top of the stack
 *
 * @note When this test pauses, widget w4 should be removed from the display.
 * All other widgets, present prior to the execution of remove_w4, should remain.
 */
void remove_w4(void) {
    delete w4;
    passOrFail(__FUNCTION__);
}

/**
 * @brief Execute each of the unity tests
 */
int runUnityTests(void) {
    UNITY_BEGIN();
    // RUN_TEST(remove_w2);
    RUN_TEST(remove_w4);
    return UNITY_END();
}

/**
 * @brief Initialization
 */
void setup() {
    Serial.begin(9600);

    // Label the test widgets.  Note: w3 already has a label; and w4 is the operator's display, not a test widget.
    w0->addItem(w0, "w0");
    w1->addItem(w1, "w1");
    w2->addItem(w2, "w2");

    // Await operator's first click on PASS or FAIL button after we've setup the display
    passOrFail("setup");

    // Execute each of the tests
    runUnityTests();
    delay(1000);
}

/**
 * @brief Cleanup after each test
 */
void tearDown(void) {
    w4->setText("");
}

// Nothing to do in loop()
void loop() {
}
