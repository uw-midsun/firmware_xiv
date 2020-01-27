#include "i2c.h"
#include "log.h"
#include "mcp23008_gpio_expander.h"
#include "mcp23008_gpio_expander_defs.h"
#include "test_helpers.h"
#include "unity.h"

// I2C address of MCP23008 + pin # that should be valid on stm32 and x86
#define VALID_I2C_ADDRESS 0x20
#define VALID_GPIO_PIN 0

// there's no invalid I2C address enforced in firmware currently
#define INVALID_GPIO_PIN (NUM_MCP23008_GPIO_PINS)

void setup_test(void) {
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,    //
    .sda = CONFIG_PIN_I2C_SDA,  //
    .scl = CONFIG_PIN_I2C_SCL,  //
  };
  return i2c_init(I2C_PORT, &i2c_settings);
}
void teardown_test(void) {}

// mcp23008_gpio_init

// Just test that init works
void test_mcp23008_gpio_init_valid(void) {
  TEST_ASSERT_OK(mcp23008_gpio_init(VALID_I2C_ADDRESS));
}

// mcp23008_gpio_init_pin

// Test that a valid configuration is ok.
void test_mcp23008_gpio_init_pin_valid(void) {
  // Default settings
  Mcp23008GpioSettings settings = {
    .direction = MCP23008_GPIO_DIR_IN,  //
    .state = MCP23008_GPIO_STATE_LOW,   //
  };
  // A pin that should be valid
  Mcp23008GpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_GPIO_PIN,             //
  };
  TEST_ASSERT_OK(mcp23008_gpio_init(VALID_I2C_ADDRESS));

  TEST_ASSERT_OK(mcp23008_gpio_init_pin(&address, &settings));
}

// Test failure when given an invalid pin number.
// Currently we don't check in firmware for a valid I2C address, so that's not tested.
void test_mcp23008_gpio_init_pin_invalid_pin_address(void) {
  // Default settings again
  Mcp23008GpioSettings settings = {
    .direction = MCP23008_GPIO_DIR_IN,  //
    .state = MCP23008_GPIO_STATE_LOW,   //
  };
  // An invalid pin
  Mcp23008GpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = INVALID_GPIO_PIN,           //
  };
  TEST_ASSERT_OK(mcp23008_gpio_init(VALID_I2C_ADDRESS));

  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, mcp23008_gpio_init_pin(&address, &settings));
}

// Test failure when given invalid settings.
void test_mcp23008_gpio_init_pin_invalid_settings(void) {
  // Some invalid settings
  Mcp23008GpioSettings settings = {
    .direction = NUM_MCP23008_GPIO_DIRS,  //
    .state = MCP23008_GPIO_STATE_LOW,     //
  };
  // A valid address
  Mcp23008GpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_GPIO_PIN,             //
  };
  TEST_ASSERT_OK(mcp23008_gpio_init(VALID_I2C_ADDRESS));

  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, mcp23008_gpio_init_pin(&address, &settings));

  settings.state = NUM_MCP23008_GPIO_STATES;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, mcp23008_gpio_init_pin(&address, &settings));

  settings.direction = MCP23008_GPIO_DIR_IN;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, mcp23008_gpio_init_pin(&address, &settings));
}

// Test that we can pick up a high state after initializing with a high state and direction out.
void test_mcp23008_gpio_init_pin_sets_output(void) {
  // Valid settings for output high
  Mcp23008GpioSettings settings = {
    .direction = MCP23008_GPIO_DIR_OUT,  //
    .state = MCP23008_GPIO_STATE_HIGH,   //
  };
  // A valid address
  Mcp23008GpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_GPIO_PIN,             //
  };

  TEST_ASSERT_OK(mcp23008_gpio_init(VALID_I2C_ADDRESS));
  TEST_ASSERT_OK(mcp23008_gpio_init_pin(&address, &settings));
  Mcp23008GpioState state = MCP23008_GPIO_STATE_LOW;

  // Check that we get a high state
  TEST_ASSERT_OK(mcp23008_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(state, MCP23008_GPIO_STATE_HIGH);
}

// mcp23008_gpio_set_state

// Comprehensive happy-path state-change test.
void test_mcp23008_gpio_set_state_valid(void) {
  // Output settings, start high
  Mcp23008GpioSettings settings = {
    .direction = MCP23008_GPIO_DIR_OUT,  //
    .state = MCP23008_GPIO_STATE_HIGH,   //
  };
  // A valid address
  Mcp23008GpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_GPIO_PIN,             //
  };

  TEST_ASSERT_OK(mcp23008_gpio_init(VALID_I2C_ADDRESS));
  TEST_ASSERT_OK(mcp23008_gpio_init_pin(&address, &settings));

  // test initializing to high
  Mcp23008GpioState state;
  TEST_ASSERT_OK(mcp23008_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_HIGH, state);

  // test changing high -> low
  TEST_ASSERT_OK(mcp23008_gpio_set_state(&address, MCP23008_GPIO_STATE_LOW));
  TEST_ASSERT_OK(mcp23008_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_LOW, state);

  // test setting it to low while already low
  TEST_ASSERT_OK(mcp23008_gpio_set_state(&address, MCP23008_GPIO_STATE_LOW));
  TEST_ASSERT_OK(mcp23008_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_LOW, state);

  // test changing low -> high
  TEST_ASSERT_OK(mcp23008_gpio_set_state(&address, MCP23008_GPIO_STATE_HIGH));
  TEST_ASSERT_OK(mcp23008_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_HIGH, state);

  // test setting it to high while already high
  TEST_ASSERT_OK(mcp23008_gpio_set_state(&address, MCP23008_GPIO_STATE_HIGH));
  TEST_ASSERT_OK(mcp23008_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_HIGH, state);
}

// Test that an invalid address is caught.
void test_mcp23008_gpio_set_state_invalid_address(void) {
  TEST_ASSERT_OK(mcp23008_gpio_init(VALID_I2C_ADDRESS));
  // An invalid address
  Mcp23008GpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = INVALID_GPIO_PIN,           //
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    mcp23008_gpio_set_state(&address, MCP23008_GPIO_STATE_LOW));
}

// Test that an invalid state is caught.
void test_mcp23008_gpio_set_state_invalid_state(void) {
  TEST_ASSERT_OK(mcp23008_gpio_init(VALID_I2C_ADDRESS));
  Mcp23008GpioAddress address = { .i2c_address = VALID_I2C_ADDRESS, .pin = VALID_GPIO_PIN };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    mcp23008_gpio_set_state(&address, NUM_MCP23008_GPIO_STATES));
}

// mcp23008_gpio_toggle_state

// Comprehensive happy-path toggle-state test.
void test_mcp23008_gpio_toggle_state(void) {
  // Valid output/low settings
  Mcp23008GpioSettings settings = {
    .direction = MCP23008_GPIO_DIR_OUT,  //
    .state = MCP23008_GPIO_STATE_LOW,    //
  };
  // A valid address
  Mcp23008GpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_GPIO_PIN,             //
  };

  TEST_ASSERT_OK(mcp23008_gpio_init(VALID_I2C_ADDRESS));
  TEST_ASSERT_OK(mcp23008_gpio_init_pin(&address, &settings));

  // test initializing to low
  Mcp23008GpioState state;
  TEST_ASSERT_OK(mcp23008_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_LOW, state);

  // test toggle low -> high
  TEST_ASSERT_OK(mcp23008_gpio_toggle_state(&address));
  TEST_ASSERT_OK(mcp23008_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_HIGH, state);

  // test toggle high -> low
  TEST_ASSERT_OK(mcp23008_gpio_toggle_state(&address));
  TEST_ASSERT_OK(mcp23008_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_LOW, state);
}

// Test that an invalid address is caught by toggle_state.
void test_mcp23008_gpio_toggle_state_invalid_address(void) {
  TEST_ASSERT_OK(mcp23008_gpio_init(VALID_I2C_ADDRESS));
  // An invalid address
  Mcp23008GpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = INVALID_GPIO_PIN,           //
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, mcp23008_gpio_toggle_state(&address));
}

// mcp23008_gpio_get_state

// Test that initializing and reading works, doubling as a test for init_pin state setting.
void test_mcp23008_gpio_get_state_valid(void) {
  // Valid output/low settings
  Mcp23008GpioSettings settings = {
    .direction = MCP23008_GPIO_DIR_OUT,  //
    .state = MCP23008_GPIO_STATE_LOW,    //
  };
  // A valid address
  Mcp23008GpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_GPIO_PIN,             //
  };

  TEST_ASSERT_OK(mcp23008_gpio_init(VALID_I2C_ADDRESS));

  // test initializing to low
  Mcp23008GpioState state = MCP23008_GPIO_STATE_HIGH;
  TEST_ASSERT_OK(mcp23008_gpio_init_pin(&address, &settings));
  TEST_ASSERT_OK(mcp23008_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_LOW, state);

  // test initializing to high
  settings.state = MCP23008_GPIO_STATE_HIGH;
  TEST_ASSERT_OK(mcp23008_gpio_init_pin(&address, &settings));
  TEST_ASSERT_OK(mcp23008_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_HIGH, state);
}

// Test that an invalid address is caught by get_state.
void test_mcp23008_gpio_get_state_invalid_address(void) {
  TEST_ASSERT_OK(mcp23008_gpio_init(VALID_I2C_ADDRESS));
  // An invalid address
  Mcp23008GpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = INVALID_GPIO_PIN,           //
  };
  Mcp23008GpioState state;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, mcp23008_gpio_get_state(&address, &state));
}
