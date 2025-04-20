
#include <Adafruit_GFX.h>

#include "DEBUG.h"
#include "HX8357_t3n.h"
#include "UserInterface.h"
// #include "UserInterfaceExterns.h"
#include "unity.h"

extern UserInterface ui;

//
void test_StationInfo(void) {
    TEST_MESSAGE("test_StationInfo()\n");
    ui.displayFrequency(7074);
    ui.displayCallsign(String("KQ7B"));
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
    ui.decodedMsgs->setItem(0, "KQ7B AG0E -13 S3", A_WHITE, A_BLACK);
}

void test_StationMsgs(void) {
    TEST_MESSAGE("test_StationMsgs\n");
    AScrollBoxItem* pCQ = ui.stationMsgs->addItem(ui.stationMsgs, "CQ KQ7B DN15");
    pCQ->setItemColors(A_YELLOW, A_BLACK);
    ui.stationMsgs->addItem(ui.stationMsgs, "KQ7B KA0XYZ DN14");
    AScrollBoxItem* pReply = ui.stationMsgs->addItem(ui.stationMsgs, "KA0XYZ KQ7B -5");
    delay(2000);
    pReply->setItemColors(A_RED, A_BLACK);
}

void test_Waterfall() {
    TEST_MESSAGE("test_Waterfall()\n");
    for (int y = 0; y < 100;y++) {
        for (int x = 0; x < 350; x++) {
            ui.theWaterfall->drawPixel(x,y,A_BLUE);
        }
    }
}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_StationInfo);
    RUN_TEST(test_DecodedMsgs);
    RUN_TEST(test_Waterfall);
    RUN_TEST(test_StationMsgs);
    return UNITY_END();
}

String foobar;

UserInterface ui;

void setup() {
    Serial.begin(9600);
    Serial.println("Starting...");

    ui.begin();

    //ATextBox foo = ATextBox("foo", 0, 200, 100, 100);
    //delay(1000);

    // Run the tests
    //delay(1000);
    runUnityTests();
    delay(1000);
}


void tearDown(void) {
    Serial.println("Finished");
}

// loop() not actually used
void loop() {}
