/**
 * @brief Exercise the UserInterface object using Unity in the PlatformIO teensy41 environment
 *
 * DISCUSSION:
 *  No, this is not a true unit test with mocks and all that... but it does execute within
 *  the Unity test framework, and it exercises the UserInterface object sans the radio code.
 *
 *  As readily feasible, this test exercises the various features of the Pocket FT8
 *  Transceiver's user interface without requiring the test operator's interactions.
 *  After completing this phase of testing within the Arduino setup() function, the test
 *  begins a second phase inside loop() where it responds to the test operator's
 *  interactions with the UI through the touchpad.  Pressing the SY button exits the test.
 */
#include <Adafruit_GFX.h>
#include <SPI.H>
#include <SD.h>

#include "DEBUG.h"
#include "HX8357_t3n.h"
#include "UserInterface.h"
#include "unity.h"
#include "Station.h"

// Build a reference to the UserInterface singleton
static UserInterface& ui = UserInterface::getInstance();  // User Interface

// Build a reference to the Station model singleton
Station& thisStation = Station::getInstance();  // Station model

/**
 * @brief Exercises the StationInfo widget
 *
 * DISCUSSION:
 *  Displays station callsign, operating frequency, offset and Transmit/Receive indicator.
 */
void test_StationInfo(void) {
    TEST_MESSAGE("test_StationInfo()\n");
    ui.displayFrequency();
    ui.displayCallsign();
    ui.setXmitRecvIndicator(INDICATOR_ICON_TUNING);
    delay(1000);
    ui.setXmitRecvIndicator(INDICATOR_ICON_RECEIVE);
    delay(1000);
    ui.setXmitRecvIndicator(INDICATOR_ICON_TRANSMIT);
    delay(1000);
    ui.setXmitRecvIndicator(INDICATOR_ICON_RECEIVE);
}

/**
 * @brief Displays a long list of "decoded" messages
 *
 * DISCUSSION:
 *  The second message, WA1ABC W1AW -2 S3, is left highlighted
 */
void test_DecodedMsgs(void) {
    TEST_MESSAGE("test_DecodedMsgs()\n");
    ui.allDecodedMsgs->setMsg(0, "WA0ABC W1AW -1 S3");
    ui.allDecodedMsgs->setMsg(1, "WA1ABC W1AW -2 S3");
    ui.allDecodedMsgs->setMsg(2, "WA2ABC W1AW -3 S3");
    ui.allDecodedMsgs->setMsg(3, "WA3ABC W1AW -4 S3");
    ui.allDecodedMsgs->setMsg(4, "WA4ABC W1AW -5 S3");
    ui.allDecodedMsgs->setMsg(5, "WA5ABC W1AW -6 S3");
    ui.allDecodedMsgs->setMsg(6, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    ui.allDecodedMsgs->setMsg(5, "THIS MESSAGE CLIPS THE RIGHTHAND SIDE!");
    ui.allDecodedMsgs->setMsg(7, "WA5ABC W1AW -9 S3");
    ui.allDecodedMsgs->setMsg(8, "WA5ABC W1AW -10 S3");
    ui.allDecodedMsgs->setMsg(9, "WA5ABC W1AW -11 S3");
    ui.allDecodedMsgs->setMsg(10, "THIS MESSAGE NOT DISPLAYED");

    delay(1000);
    ui.allDecodedMsgs->setItemColor(1, A_BLACK, A_LIGHT_GREY);  // Highlight item 1
}

void test_popup(void) {
    TEST_MESSAGE("test_popup()\n");
    delay(1000);
    ATextBox* popup = new ATextBox("I am the popup widget", 0, 0, 440, 310, A_RED);
    delay(1000);   // Leave popup on the screen for a bit
    delete popup;  // Should repaint overlapping widgets
    delay(1000);   // Leave repainted gui visible for a moment
}

void test_StationMsgs(void) {
    Decode msg1, msg2;
    strcpy(msg1.field1, "CQ");
    strcpy(msg1.field2, "W1AW");
    strcpy(msg1.field3, "CM13");
    strcpy(msg2.field1, "W1AW");
    strcpy(msg2.field2, "KQ7B");
    strcpy(msg2.field3, "DN15");
    TEST_MESSAGE("test_StationMsgs\n");
    QSOMessagesItem* pCQ = ui.theQSOMsgs->addStationMessageItem(ui.theQSOMsgs, &msg1, QSO_MSG_RECVD);
    pCQ->setItemColors(A_YELLOW, A_BLACK);
    ui.theQSOMsgs->addStationMessageItem(ui.theQSOMsgs, &msg2, QSO_MSG_XMITD);
    ui.theQSOMsgs->addStationMessageItem(ui.theQSOMsgs, "NA1A KQ7B 73", QSO_MSG_XMITPEND);
    ui.theQSOMsgs->addItem(ui.theQSOMsgs, "1234567890ABCDEFGHIJKLMNOP");  // Invoke base class addItem to display long undecoded string
    delay(1000);
    AWidget::processTouch(270, 120);  // Touch W1AW's CQ message
    delay(1000);
}  // test_StationMsgs()

static unsigned cursorX = 102;
void test_Waterfall() {
    TEST_MESSAGE("test_Waterfall()\n");
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 350; x++) {
            // ui.theWaterfall->drawPixel(x, y, A_BLUE);
            if (x == cursorX) ui.theWaterfall->drawPixel(x, y, A_RED);
        }
    }
}  // test_Waterfall()

void test_AppMessage() {
    TEST_MESSAGE("test_AppMessage\n");
    ui.applicationMsgs->setText("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");
    delay(1000);
}

void test_CQButton() {
    TEST_MESSAGE("test_CQButton\n");
    TEST_ASSERT(ui.b0->getUserData() == 0);
    TEST_ASSERT_FALSE(ui.b0->getState());
    AWidget::processTouch(30, 300);  // Toggle button "on"
    TEST_ASSERT(ui.b0->getState());
    delay(1000);                     // Wait a bit
    AWidget::processTouch(30, 300);  // Toggle button "off"
    TEST_ASSERT_FALSE(ui.b0->getState());
}

void test_AbortButton() {
    char s[256];
    TEST_MESSAGE("test_AbortButton\n");
    sprintf(s, "userData=%d\n", ui.b1->getUserData());
    TEST_ASSERT(ui.b1->getUserData() == 1);
    TEST_ASSERT_FALSE(ui.b1->getState());
    AWidget::processTouch(90, 300);  // Toggle button "on"
    TEST_ASSERT(ui.b1->getState());
    delay(1000);                     // Wait a bit
    AWidget::processTouch(90, 300);  // Toggle button "off"
    TEST_ASSERT_FALSE(ui.b1->getState());
}

extern void process_touch(void);
static unsigned long t0;
/**
 * @brief Begin interactive touchscreen test
 *
 * DISCUSSION:
 *  This test mostly executes in loop() polling for and responding to touchscreen events
 */
void test_TouchScreen() {
    TEST_MESSAGE("Press SY to exit test_UserInterface");
    ui.applicationMsgs->setText("TouchScreen test");
    t0 = millis();
}

static bool inLoop = false;  // Test code in loop()
/**
 * @brief Callback function invoked by UserInterace MenuButton::onTouchButton()
 * @param button Pointer to this button
 * @param buttonId UserInterface's id number for this pressed button
 */
void buttonPressCallback(MenuButton* button, int buttonId) {
    delay(100);
    DTRACE();
    if (inLoop) button->reset();  // Reset button following setup()

    // Did test operator request the test to end?
    if (buttonId == 7) {
        DTRACE();
        UNITY_END();
    }
}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_StationInfo);
    RUN_TEST(test_DecodedMsgs);
    RUN_TEST(test_Waterfall);
    RUN_TEST(test_StationMsgs);
    RUN_TEST(test_AppMessage);
    RUN_TEST(test_CQButton);
    RUN_TEST(test_AbortButton);
    RUN_TEST(test_popup);
    RUN_TEST(test_TouchScreen);
    return /*UNITY_END()*/;  // UserInterface.cpp invokes UNITY_END()
}

String foobar;

void setup() {
    Serial.begin(9600);
    if (!Serial) delay(100);

    // Mock the Station assembly
    thisStation.setCallsign("AA0A");
    thisStation.setLocator("DN15");
    thisStation.setRig("TX1");
    thisStation.setMyName("Joe");
    thisStation.setFrequency(7074);
    thisStation.setCursorFreq(1000);

    // Get the SD file system running
    if (!SD.begin(BUILTIN_SDCARD)) {
        DTRACE();
    }

    ui.begin();

    // Run the tests
    runUnityTests();
    delay(1000);

    ui.allDecodedMsgs->setItemColor(1, A_WHITE, A_BLACK);  // Restore above highlighted test item's color scheme
}

void tearDown(void) {
    Serial.println("Finished");
}

/**
 * @brief The Great Arduino scheduler loop
 *
 * DISCUSSION:
 *  Testing uses loop() to poll the touchscreen and respond to touch events through
 *  the User Interface for timout milliseconds
 */
static unsigned long timeout = 10000;  // Milliseconds till timeout
void loop() {
    // Time-out???
    if (millis() - t0 > timeout) UNITY_END();

    // Interactive touchscreen testing mode
    inLoop = true;  // Test is now in interactive mode responding to operator touch events
    pollTouchscreen();
}
