
#include <Adafruit_GFX.h>

#include "DEBUG.h"
#include "HX8357_t3n.h"
#include "UserInterface.h"
// #include "UserInterfaceExterns.h"
#include "unity.h"

//
void test_01(void) {
    TEST_MESSAGE("foo...");
}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_01);
    return UNITY_END();
}

String foobar;

UserInterface ui;

void setup() {
    Serial.begin(9600);
    Serial.println("Starting...");

    ui.begin();

    ATextBox foo = ATextBox("foo", 0, 200, 100, 100);
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
