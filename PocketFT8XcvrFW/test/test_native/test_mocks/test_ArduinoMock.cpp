/**
 * test_time exercises the ft8_lib interface
 */

#include <unity.h>
#include "Arduino.h"
#include "DEBUG.h"
#include <string>

std::string foobar;

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

void test_debugio(void) {
    DPRINTF("DPRINTF is working\n");
}

/**
 * @brief This is the Arduino setup() function invoked when program starts
 */
int main(int argc, char** argv) {
    // Initialize unity
    UNITY_BEGIN();

    // Run the tests
    RUN_TEST(test_debugio);

    // Finished
    UNITY_END();
}

/**
 * @brief Nothing to do in the Arduino loop() function
 */
void loop() {
    // Not used
}