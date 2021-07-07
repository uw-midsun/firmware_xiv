#include "mci_fan_control.h"

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "ms_test_helpers.h"

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
}

void teardown_test(void) {}

// Initialize fan control with no callbacks.
static void prv_init_fan(void) {
  MciFanControlSettings settings = {
    .fault_cb = NULL,
    .fault_context = NULL,
  };
  TEST_ASSERT_OK(mci_fan_control_init(&settings));
}

// Used to make sure the fault callback is working as expected
static uint16_t s_times_cb_called;
static uint8_t s_fault_bitset;
static void prv_test_fault_cb(uint8_t fault_bitset, void *context) {
  s_times_cb_called++;
  s_fault_bitset = fault_bitset;
}

// Initialize fan control with prv_test_fault_cb
static void prv_init_fan_with_cb(void) {
  MciFanControlSettings settings = {
    .fault_cb = prv_test_fault_cb,
    .fault_context = NULL,
  };
  TEST_ASSERT_OK(mci_fan_control_init(&settings));
}

// Confirm that the fan state is as expected.
static void prv_assert_fan_state(GpioState state) {
  GpioAddress test_en_addr = MCI_FAN_EN_ADDR;
  GpioState test_en_state = GPIO_STATE_LOW;
  TEST_ASSERT_OK(gpio_get_state(&test_en_addr, &test_en_state));
  TEST_ASSERT_EQUAL(state, test_en_state);
}

// Confirm that fan control initializes as expected.
void test_init(void) {
  prv_init_fan();

  // Pin should be high after initialization
  prv_assert_fan_state(GPIO_STATE_HIGH);
}

// Confirm that the fan can be turned on and off.
void test_set_state(void) {
  prv_init_fan();

  mci_fan_set_state(MCI_FAN_STATE_OFF);
  prv_assert_fan_state(GPIO_STATE_LOW);

  mci_fan_set_state(MCI_FAN_STATE_ON);
  prv_assert_fan_state(GPIO_STATE_HIGH);
}

// Initialize with a callback.
void test_init_with_cb(void) {
  prv_init_fan_with_cb();
}
// Confirm that general fault handling procedures work as expected
void test_general_fault(void) {
  prv_init_fan_with_cb();

  TEST_ASSERT_EQUAL(0, s_times_cb_called);
  TEST_ASSERT_EQUAL(0, s_fault_bitset);

  // Q1 overtemp
  GpioAddress test_pin = MCI_Q1_OVERTEMP_ADDR;
  gpio_set_state(&test_pin, GPIO_STATE_HIGH);
  gpio_it_trigger_interrupt(&test_pin);
  TEST_ASSERT_EQUAL(1, s_times_cb_called);
  TEST_ASSERT_EQUAL((1 << MCI_THERM_Q1_OVERTEMP), s_fault_bitset);
}
