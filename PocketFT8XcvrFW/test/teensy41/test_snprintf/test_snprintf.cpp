#include <Arduino.h>
#include <unity.h>

void setUp(void) {
}

void tearDown(void) {
}

void test_negatives(void) {
    TEST_MESSAGE("test_negatives");
    char message[18];
    int n;

    n = -1;
    snprintf(message, sizeof(message), "%s %s S%3i", "KI0E", "KQ7B", n);
    TEST_MESSAGE(message);

    n = -10;
    snprintf(message, sizeof(message), "%s %s S%3i", "KI0E", "KQ7B", n);
    TEST_MESSAGE(message);

    n = -100;
    snprintf(message, sizeof(message), "%s %s S%3i", "KI0E", "KQ7B", n);
    TEST_MESSAGE(message);

    n = -1000;
    snprintf(message, sizeof(message), "%s %s S%3i", "KI0E", "KQ7B", n);
    TEST_MESSAGE(message);

    n = -100000;
    snprintf(message, sizeof(message), "%s %s S%3i", "KI0E", "KQ7B", n);
    TEST_MESSAGE(message);

    n = -1000000;
    snprintf(message, sizeof(message), "%s %s S%3i", "KI0E", "KQ7B", n);
    TEST_MESSAGE(message);

    n = -10000000;
    snprintf(message, sizeof(message), "%s %s S%3i", "KI0E", "KQ7B", n);
    TEST_MESSAGE(message);
}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_negatives);
    // RUN_TEST(test_rig);
    return UNITY_END();
}

void setup() {
    Serial.begin(9600);

    delay(1000);

    // Run the tests
    runUnityTests();

    delay(10000);
}

void loop() {
    // delay(1000);
}