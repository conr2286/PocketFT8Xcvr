
#include <Adafruit_GFX.h>

#include "DEBUG.h"
#include "HX8357_t3n.h"
#include "UserInterface.h"
#include "unity.h"
#include "Station.h"

extern UserInterface ui;

Station thisStation;

//
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

void test_DecodedMsgs(void) {
    TEST_MESSAGE("test_DecodedMsgs()\n");
    ui.decodedMsgs->setMsg(0, "WA0ABC AG0E -13 S3");
    ui.decodedMsgs->setMsg(1, "WA1ABC AG0E -13 S3");
    ui.decodedMsgs->setMsg(2, "WA2ABC AG0E -13 S3");
    ui.decodedMsgs->setMsg(3, "WA3ABC AG0E -13 S3");
    ui.decodedMsgs->setMsg(4, "WA4ABC AG0E -13 S3");
    ui.decodedMsgs->setMsg(5, "WA5ABC AG0E -13 S3");
    delay(1000);
    // AWidget::processTouch(20, 124); //Highlight item 0
    delay(1000);
    ui.decodedMsgs->setItemColor(1, A_BLACK, A_LIGHT_GREY);  // Highlight item 1
}

void test_popup(void) {
    TEST_MESSAGE("test_popup()\n");
    delay(1000);
    ATextBox* popup = new ATextBox("I am the popup widget", 0, 0, 440, 310, A_RED);
    delay(2000);   // Leave popup on the screen for a bit
    delete popup;  // Should repaint overlapping widgets
    delay(2000);   // Leave repainted gui visible for a moment
}

void test_StationMsgs(void) {
    Decode msg1, msg2;
    strcpy(msg1.field1, "CQ");
    strcpy(msg1.field2, "AG0E");
    strcpy(msg1.field3, "CM13");
    strcpy(msg2.field1, "AG0E");
    strcpy(msg2.field2, "KQ7B");
    strcpy(msg2.field3, "DN15");
    TEST_MESSAGE("test_StationMsgs\n");
    StationMessagesItem* pCQ = ui.stationMsgs->addStationMessageItem(ui.stationMsgs, &msg1);
    pCQ->setItemColors(A_YELLOW, A_BLACK);
    ui.stationMsgs->addStationMessageItem(ui.stationMsgs, &msg2);
    ui.stationMsgs->addStationMessageItem(ui.stationMsgs, "NA1A KQ7B 73");
    delay(2000);
    AWidget::processTouch(270, 120);  // Touch AG0E's CQ message
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
    delay(5000);
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
void test_TouchScreen() {
    TEST_MESSAGE("Touch any displayed button in the next 15 seconds");
    for (int i = 0; i < 150; i++) {
        pollTouchscreen();
        delay(100);
    }
    TEST_MESSAGE("Finished\n");
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
    return UNITY_END();
}

String foobar;

UserInterface ui;

void setup() {
    Serial.begin(9600);

    // Mock the Station assembly
    thisStation.setCallsign("AA0A");
    thisStation.setLocator("BB0B");
    thisStation.setRig("TX1");
    thisStation.setMyName("CCCC");
    thisStation.setFrequency(1234);
    thisStation.setCursorFreq(555);

    ui.begin();

    // ATextBox foo = ATextBox("foo", 0, 200, 100, 100);
    // delay(1000);

    // Run the tests
    // delay(1000);
    runUnityTests();
    delay(1000);
}

void tearDown(void) {
    Serial.println("Finished");
}

// loop() not actually used
void loop() {}
