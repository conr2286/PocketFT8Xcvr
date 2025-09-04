/**
 * test_time exercises the various Teensy 4.1 time sources
 */

#include <Arduino.h>
#include <unity.h>
#include <TimeLib.h>

/**
 * @brief This is the unity setup method executed prior to each test
 */
void setUp(void) {
}

/**
 * @brief This is the unity tearDown method executed following each test
 * @param
 */
void tearDown(void) {
}

/**
 * @brief Confirm that hour():minute():second() are in sync with now()
 */
void test_time_sources(void) {
    // Get the current time as hh:mn:ss mh:dy:yy
    TimeElements t1;
    t1.Hour = hour();
    t1.Minute = minute();
    t1.Second = second();
    t1.Month = month();
    t1.Day = day();
    t1.Year = CalendarYrToTm(year());
    Serial.printf("year()=%d\n", year());
    time_t et1 = makeTime(t1);

    // Get the seconds elapsed from midnight 1/1/1970
    time_t et2 = now();

    // Display t1 times for debugging
    Serial.printf("t1 HH:MM:SS = %02d:%02d:%02d\n", t1.Hour, t1.Minute, t1.Second);
    Serial.printf("t1 MM:DD:YY = %02d/%02d/%02d\n", t1.Month, t1.Day, t1.Year);

    // Display t2 times for debugging
    Serial.printf("et2 HH:MM:SS = %02d:%02d:%02d\n", hour(et2), minute(et2), second(et2));
    Serial.printf("et2 MM:DD:YY = %02d/%02d/%02d\n", month(et2), day(et2), year(et2));

    // Display elapsed time diffs
    Serial.printf("et1=%lu, et2=%lu\n", et1, et2);

    // Verify the elapsed time is not approximately zero (i.e. verify some reasonable elapsed time)
    TEST_ASSERT_GREATER_OR_EQUAL_INT32_MESSAGE(100, et1, "Uninitialized elapsed time et1");
    TEST_ASSERT_GREATER_OR_EQUAL_INT32_MESSAGE(100, et2, "Uninitialized elapsed time et2");

    // Verify the two time sources are approximately equal
    TEST_ASSERT_LESS_OR_EQUAL_UINT32_MESSAGE(1, et2 - et1, "Time sources do not agree");
}

/**
 * @brief This is the Arduino setup() function executed when program starts
 */
void setup() {
    Serial.begin(9600);
    delay(1000);

    // Initialize unity
    UNITY_BEGIN();

    // Run the tests
    RUN_TEST(test_time_sources);

    // Finished
    // UNITY_END();
}

/**
 * @brief Nothing to do in the Arduino loop() function
 */
void loop() {
    Serial.printf("loop()ing\n");
    RUN_TEST(test_time_sources);
    UNITY_END();
}