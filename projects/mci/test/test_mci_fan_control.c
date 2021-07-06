#include "mci_fan_control.h"

#include "gpio.h"
#include "ms_test_helpers.h"

void setup_test(void) {
  gpio_init();
}

void teardown_test(void) {}

void test_init(void) {
  MciFanControlSettings settings = {
    .fault_cb = NULL,
    .fault_context = NULL,
  };
  TEST_ASSERT_OK(mci_fan_control_init(&settings));

  // Pin should be high after initialization
  GpioAddress test_en_addr = MCI_FAN_EN_ADDR;
  GpioState test_en_state = GPIO_STATE_LOW;
  TEST_ASSERT_OK(gpio_get_state(&test_en_addr, &test_en_state));
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, test_en_state);
}
