#include "mci_fan_control.h"

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "ms_test_helpers.h"

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

static GpioState s_test_get_state;
StatusCode TEST_MOCK(gpio_get_state)(const GpioAddress *address, GpioState *input_state) {
  *input_state = s_test_get_state;
  return STATUS_CODE_OK;
}

// Only used to check EN pin
static GpioState s_test_en_set_state;
static const GpioAddress s_en_pin_addr = MCI_FAN_EN_ADDR;
StatusCode TEST_MOCK(gpio_set_state)(const GpioAddress *address, GpioState state) {
  if (address->pin == s_en_pin_addr.pin) {
    s_test_en_set_state = state;
  }
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
}

void teardown_test(void) {}

// Confirm that fan control initializes as expected.
void test_init(void) {
  prv_init_fan();

  // Pin should be high after initialization
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_en_set_state);
}

// Confirm that the fan can be turned on and off.
void test_set_state(void) {
  prv_init_fan();

  mci_fan_set_state(MCI_FAN_STATE_OFF);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_test_en_set_state);

  mci_fan_set_state(MCI_FAN_STATE_ON);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_en_set_state);
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
  s_test_get_state = GPIO_STATE_HIGH;
  gpio_it_trigger_interrupt(&test_pin);
  TEST_ASSERT_EQUAL(1, s_times_cb_called);
  TEST_ASSERT_EQUAL((1 << MCI_THERM_Q1_OVERTEMP), s_fault_bitset);
}
