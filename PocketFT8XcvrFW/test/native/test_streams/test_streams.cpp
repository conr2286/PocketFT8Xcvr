#include <unity.h>
#include <stdint.h>

#include "CommChannel.h"

void setUp(void)
{
  // set stuff up here
}

void tearDown(void)
{
  // clean stuff up here
}

void test_led_builtin_pin_number(void)
{
  //TEST_ASSERT_EQUAL(13, LED_BUILTIN);
}


int main(void) {

  UNITY_BEGIN();  // IMPORTANT LINE!
  RUN_TEST(test_led_builtin_pin_number);


}
