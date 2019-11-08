#include <stdint.h>

#include "gpio.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {}

void teardown_test(void) {}

#define VALID_PIN 0
#define VALID_PORT 0

#define INVALID_PIN (GPIO_PINS_PER_PORT)
#define INVALID_PORT (NUM_GPIO_PORTS)

// Used to impose a delay to due to the 12 MHz max slew rate of the GPIO on the
// stm32f0xx.
void delay(void) {
  for (volatile int16_t i = 0; i < 1000; i++) {
  }
}

// gpio_init

// Tests that the GPIO init works. This should simply return true
// INVALID_PORT_PIN% of time unless the test is on x86 and the configuration is
// incorrect in which case fix it!
void test_gpio_init_valid(void) {
  TEST_ASSERT_OK(gpio_init());
}

// gpio_init_pin

// Test that a valid gpio configuration will work.
void test_gpio_init_pin_valid(void) {
  // Default settings for a pin.
  GpioSettings settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_PULLUP,
    .alt_function = GPIO_ALTFN_NONE,
  };
  // A pin that should be valid on all configurations.
  GpioAddress address = {
    .port = VALID_PORT,  //
    .pin = VALID_PIN     //
  };
  TEST_ASSERT_OK(gpio_init());
  TEST_ASSERT_OK(gpio_init_pin(&address, &settings));
}

// Tests a set of addresses far outside normal range.
void test_gpio_init_pin_invalid_address(void) {
  // Default settings for a pin.
  GpioSettings settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_PULLUP,
    .alt_function = GPIO_ALTFN_NONE,
  };
  TEST_ASSERT_OK(gpio_init());
  // A port that should be invalid on all configurations.
  GpioAddress address = {
    .port = INVALID_PORT,  //
    .pin = VALID_PIN       //
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_init_pin(&address, &settings));
  address.pin = INVALID_PIN;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_init_pin(&address, &settings));
  address.port = VALID_PORT;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_init_pin(&address, &settings));
}

// Tests a set of settings outside normal range.
void test_gpio_init_pin_invalid_settings(void) {
  // Bad settings for a pin.
  GpioSettings settings = {
    .direction = NUM_GPIO_DIRS,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_PULLUP,
    .alt_function = GPIO_ALTFN_NONE,
  };
  TEST_ASSERT_OK(gpio_init());
  // A port that should be valid on all configurations.
  GpioAddress address = {
    .port = VALID_PORT,  //
    .pin = VALID_PIN     //
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_init_pin(&address, &settings));
  settings.direction = GPIO_DIR_IN;
  settings.state = NUM_GPIO_STATES;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_init_pin(&address, &settings));
  settings.state = GPIO_STATE_LOW;
  settings.resistor = NUM_GPIO_RESES;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_init_pin(&address, &settings));
  settings.resistor = GPIO_RES_NONE;
  settings.alt_function = NUM_GPIO_ALTFNS;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_init_pin(&address, &settings));
}

// Tests that if the pin is set to direction out a high state will be picked up
// in the input register.
void test_gpio_init_pin_valid_output(void) {
  // Default high settings for a pin.
  GpioSettings settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  // A pin that should be valid on all configurations.
  GpioAddress address = {
    .port = VALID_PORT,  //
    .pin = VALID_PIN     //
  };
  TEST_ASSERT_OK(gpio_init());
  TEST_ASSERT_OK(gpio_init_pin(&address, &settings));
  GpioState state = GPIO_STATE_LOW;
  // Check high
  TEST_ASSERT_OK(gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);
}

// TODO(ELEC-32): Figure out how to verify: PUPDR, ALTFN.

// gpio_set_state

// Test that a valid state change will work.
void test_gpio_set_state_valid(void) {
  // Default output settings for a pin.
  GpioSettings settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_PULLUP,
    .alt_function = GPIO_ALTFN_NONE,
  };
  // A pin that should be valid on all boards.
  GpioAddress address = {
    .port = VALID_PORT,  //
    .pin = VALID_PIN     //
  };
  TEST_ASSERT_OK(gpio_init());
  TEST_ASSERT_OK(gpio_init_pin(&address, &settings));
  GpioState state;
  // ON to OFF
  TEST_ASSERT_OK(gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);
  TEST_ASSERT_OK(gpio_set_state(&address, GPIO_STATE_LOW));
  delay();
  TEST_ASSERT_OK(gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, state);
  // OFF to ON
  TEST_ASSERT_OK(gpio_set_state(&address, GPIO_STATE_HIGH));
  delay();
  TEST_ASSERT_OK(gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);
}

// Test that an invalid address is caught.
void test_gpio_set_state_invalid_address(void) {
  TEST_ASSERT_OK(gpio_init());
  // A port that should be invalid on all configurations.
  GpioAddress address = { .port = INVALID_PORT, .pin = VALID_PIN };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_set_state(&address, GPIO_STATE_LOW));
  address.pin = INVALID_PIN;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_set_state(&address, GPIO_STATE_LOW));
  address.port = VALID_PORT;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_set_state(&address, GPIO_STATE_LOW));
}

// Test that an invalid state is caught.
void test_gpio_set_state_invalid_state(void) {
  TEST_ASSERT_OK(gpio_init());
  // A port that should be valid on all configurations.
  GpioAddress address = { .port = VALID_PORT, .pin = VALID_PIN };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_set_state(&address, NUM_GPIO_STATES));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_set_state(&address, (GpioState)-1));
}

// gpio_toggle_state

// Test that a valid state toggle will work.
void test_gpio_toggle_state_valid(void) {
  // Default output settings for a pin.
  GpioSettings settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_PULLUP,
    .alt_function = GPIO_ALTFN_NONE,
  };
  // A pin that should be valid on all boards.
  GpioAddress address = {
    .port = VALID_PORT,  //
    .pin = VALID_PIN     //
  };
  TEST_ASSERT_OK(gpio_init());
  TEST_ASSERT_OK(gpio_init_pin(&address, &settings));
  GpioState state;
  // OFF to ON
  TEST_ASSERT_OK(gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, state);
  TEST_ASSERT_OK(gpio_toggle_state(&address));
  delay();
  TEST_ASSERT_OK(gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);
  // ON to OFF
  TEST_ASSERT_OK(gpio_toggle_state(&address));
  delay();
  TEST_ASSERT_OK(gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, state);
}

// Test that an invalid address is caught.
void test_gpio_toggle_state_invalid_address(void) {
  TEST_ASSERT_OK(gpio_init());
  // A port that should be invalid on all configurations.
  GpioAddress address = {
    .port = INVALID_PORT,  //
    .pin = VALID_PIN       //
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_toggle_state(&address));
  address.pin = INVALID_PIN;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_toggle_state(&address));
  address.port = VALID_PORT;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_toggle_state(&address));
}

// gpio_get_state

// Test that a valid get value will work. This doubles as a test to verify that
// the state setting on gpio_init_pin works.
void test_gpio_get_state_valid(void) {
  // Default output settings for a pin.
  GpioSettings settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_PULLUP,
    .alt_function = GPIO_ALTFN_NONE,
  };
  // A pin that should be valid on all boards.
  GpioAddress address = {
    .port = VALID_PORT,  //
    .pin = VALID_PIN     //
  };
  TEST_ASSERT_OK(gpio_init());
  TEST_ASSERT_OK(gpio_init_pin(&address, &settings));
  GpioState state;
  // Get Low
  TEST_ASSERT_OK(gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, state);
  settings.state = GPIO_STATE_HIGH;
  // Get High
  TEST_ASSERT_OK(gpio_init_pin(&address, &settings));
  TEST_ASSERT_OK(gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);
}

// Test that an invalid address is caught.
void test_gpio_get_state_invalid_address(void) {
  TEST_ASSERT_OK(gpio_init());
  // A port that should be invalid on all configurations.
  GpioAddress address = {
    .port = INVALID_PORT,  //
    .pin = VALID_PIN       //
  };
  GpioState state;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_get_state(&address, &state));
  address.pin = INVALID_PIN;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_get_state(&address, &state));
  address.port = VALID_PORT;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_get_state(&address, &state));
}
