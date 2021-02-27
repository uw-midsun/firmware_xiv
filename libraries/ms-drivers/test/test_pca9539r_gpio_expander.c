#include "i2c.h"
#include "log.h"
#include "pca9539r_gpio_expander.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_I2C_PORT I2C_PORT_2

#define TEST_CONFIG_PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define TEST_CONFIG_PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

// I2C address of PCA9539R + pins that should be valid on stm32 and x86
#define VALID_I2C_ADDRESS 0x74
#define VALID_PORT_0_PIN PCA9539R_PIN_IO0_7
#define VALID_PORT_1_PIN PCA9539R_PIN_IO1_0

// there's no invalid I2C address enforced in firmware currently
#define INVALID_GPIO_PIN (NUM_PCA9539R_GPIO_PINS)

void setup_test(void) {
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,         //
    .sda = TEST_CONFIG_PIN_I2C_SDA,  //
    .scl = TEST_CONFIG_PIN_I2C_SCL,  //
  };
  i2c_init(TEST_I2C_PORT, &i2c_settings);
  gpio_it_init();
}

void teardown_test(void) {}

// pca9539r_gpio_init

// Just test that init works
void test_pca9539r_gpio_init_valid(void) {
  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, VALID_I2C_ADDRESS));
}

// pca9539r_gpio_init_pin

// Test that a valid configuration is ok.
void test_pca9539r_gpio_init_pin_valid(void) {
  // Default settings
  Pca9539rGpioSettings settings = {
    .direction = PCA9539R_GPIO_DIR_IN,  //
    .state = PCA9539R_GPIO_STATE_LOW,   //
  };
  // A pin that should be valid
  Pca9539rGpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_PORT_0_PIN,           //
  };
  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, VALID_I2C_ADDRESS));

  TEST_ASSERT_OK(pca9539r_gpio_init_pin(&address, &settings));
}

// Test failure when given an invalid pin number.
// Currently we don't check in firmware for a valid I2C address, so that's not tested.
void test_pca9539r_gpio_init_pin_invalid_pin_address(void) {
  // Default settings again
  Pca9539rGpioSettings settings = {
    .direction = PCA9539R_GPIO_DIR_IN,  //
    .state = PCA9539R_GPIO_STATE_LOW,   //
  };
  // An invalid pin
  Pca9539rGpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = INVALID_GPIO_PIN,           //
  };
  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, VALID_I2C_ADDRESS));

  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pca9539r_gpio_init_pin(&address, &settings));
}

// Test failure when given invalid settings.
void test_pca9539r_gpio_init_pin_invalid_settings(void) {
  // Some invalid settings
  Pca9539rGpioSettings settings = {
    .direction = NUM_PCA9539R_GPIO_DIRS,  //
    .state = PCA9539R_GPIO_STATE_LOW,     //
  };
  // A valid address
  Pca9539rGpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_PORT_0_PIN,           //
  };
  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, VALID_I2C_ADDRESS));

  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pca9539r_gpio_init_pin(&address, &settings));

  settings.state = NUM_PCA9539R_GPIO_STATES;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pca9539r_gpio_init_pin(&address, &settings));

  settings.direction = PCA9539R_GPIO_DIR_IN;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pca9539r_gpio_init_pin(&address, &settings));
}

// Test that we can pick up a high state after initializing with a high state and direction out.
void test_pca9539r_gpio_init_pin_sets_output(void) {
  // Valid settings for output high
  Pca9539rGpioSettings settings = {
    .direction = PCA9539R_GPIO_DIR_OUT,  //
    .state = PCA9539R_GPIO_STATE_HIGH,   //
  };
  // A valid address
  Pca9539rGpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_PORT_0_PIN,           //
  };

  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, VALID_I2C_ADDRESS));
  TEST_ASSERT_OK(pca9539r_gpio_init_pin(&address, &settings));
  Pca9539rGpioState state = PCA9539R_GPIO_STATE_LOW;

  // Check that we get a high state
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(state, PCA9539R_GPIO_STATE_HIGH);
}

// pca9539r_gpio_set_state

// Comprehensive happy-path state-change test.
void test_pca9539r_gpio_set_state_valid(void) {
  // Output settings, start high
  Pca9539rGpioSettings settings = {
    .direction = PCA9539R_GPIO_DIR_OUT,  //
    .state = PCA9539R_GPIO_STATE_HIGH,   //
  };
  // A valid address
  Pca9539rGpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_PORT_0_PIN,           //
  };

  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, VALID_I2C_ADDRESS));
  TEST_ASSERT_OK(pca9539r_gpio_init_pin(&address, &settings));

  // test initializing to high
  Pca9539rGpioState state;
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_HIGH, state);

  // test changing high -> low
  TEST_ASSERT_OK(pca9539r_gpio_set_state(&address, PCA9539R_GPIO_STATE_LOW));
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_LOW, state);

  // test setting it to low while already low
  TEST_ASSERT_OK(pca9539r_gpio_set_state(&address, PCA9539R_GPIO_STATE_LOW));
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_LOW, state);

  // test changing low -> high
  TEST_ASSERT_OK(pca9539r_gpio_set_state(&address, PCA9539R_GPIO_STATE_HIGH));
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_HIGH, state);

  // test setting it to high while already high
  TEST_ASSERT_OK(pca9539r_gpio_set_state(&address, PCA9539R_GPIO_STATE_HIGH));
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_HIGH, state);

  // test that a port 1 address also works
  // A valid port-1 address
  Pca9539rGpioAddress address_port_1 = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_PORT_1_PIN,           //
  };
  TEST_ASSERT_OK(pca9539r_gpio_init_pin(&address_port_1, &settings));
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address_port_1, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_HIGH, state);
  TEST_ASSERT_OK(pca9539r_gpio_set_state(&address_port_1, PCA9539R_GPIO_STATE_LOW));
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address_port_1, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_LOW, state);
}

// Test that an invalid address is caught.
void test_pca9539r_gpio_set_state_invalid_address(void) {
  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, VALID_I2C_ADDRESS));
  // An invalid address
  Pca9539rGpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = INVALID_GPIO_PIN,           //
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    pca9539r_gpio_set_state(&address, PCA9539R_GPIO_STATE_LOW));
}

// Test that an invalid state is caught.
void test_pca9539r_gpio_set_state_invalid_state(void) {
  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, VALID_I2C_ADDRESS));
  Pca9539rGpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_PORT_0_PIN,           //
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    pca9539r_gpio_set_state(&address, NUM_PCA9539R_GPIO_STATES));
}

// pca9539r_gpio_toggle_state

// Comprehensive happy-path toggle-state test.
void test_pca9539r_gpio_toggle_state(void) {
  // Valid output/low settings
  Pca9539rGpioSettings settings = {
    .direction = PCA9539R_GPIO_DIR_OUT,  //
    .state = PCA9539R_GPIO_STATE_LOW,    //
  };
  // A valid address
  Pca9539rGpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_PORT_1_PIN,           //
  };

  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, VALID_I2C_ADDRESS));
  TEST_ASSERT_OK(pca9539r_gpio_init_pin(&address, &settings));

  // test initializing to low
  Pca9539rGpioState state;
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_LOW, state);

  // test toggle low -> high
  TEST_ASSERT_OK(pca9539r_gpio_toggle_state(&address));
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_HIGH, state);

  // test toggle high -> low
  TEST_ASSERT_OK(pca9539r_gpio_toggle_state(&address));
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_LOW, state);

  // test that a port 1 address also works
  // A valid port 1 address
  Pca9539rGpioAddress address_port_1 = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_PORT_1_PIN,           //
  };
  TEST_ASSERT_OK(pca9539r_gpio_init_pin(&address_port_1, &settings));
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address_port_1, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_LOW, state);
  TEST_ASSERT_OK(pca9539r_gpio_toggle_state(&address_port_1));
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address_port_1, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_HIGH, state);
}

// Test that an invalid address is caught by toggle_state.
void test_pca9539r_gpio_toggle_state_invalid_address(void) {
  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, VALID_I2C_ADDRESS));
  // An invalid address
  Pca9539rGpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = INVALID_GPIO_PIN,           //
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pca9539r_gpio_toggle_state(&address));
}

// pca9539r_gpio_get_state

// Test that initializing and reading works, doubling as a test for init_pin state setting.
void test_pca9539r_gpio_get_state_valid(void) {
  // Valid output/low settings
  Pca9539rGpioSettings settings = {
    .direction = PCA9539R_GPIO_DIR_OUT,  //
    .state = PCA9539R_GPIO_STATE_LOW,    //
  };
  // A valid address
  Pca9539rGpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_PORT_0_PIN,           //
  };

  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, VALID_I2C_ADDRESS));

  // test initializing to low
  Pca9539rGpioState state = PCA9539R_GPIO_STATE_HIGH;
  TEST_ASSERT_OK(pca9539r_gpio_init_pin(&address, &settings));
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_LOW, state);

  // test initializing to high
  settings.state = PCA9539R_GPIO_STATE_HIGH;
  TEST_ASSERT_OK(pca9539r_gpio_init_pin(&address, &settings));
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_HIGH, state);

  // test that a port 1 address also works
  // A valid port 1 address
  Pca9539rGpioAddress address_port_1 = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = VALID_PORT_1_PIN,           //
  };
  settings.state = PCA9539R_GPIO_STATE_LOW;
  TEST_ASSERT_OK(pca9539r_gpio_init_pin(&address_port_1, &settings));
  TEST_ASSERT_OK(pca9539r_gpio_get_state(&address_port_1, &state));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_LOW, state);
}

// Test that an invalid address is caught by get_state.
void test_pca9539r_gpio_get_state_invalid_address(void) {
  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, VALID_I2C_ADDRESS));
  // An invalid address
  Pca9539rGpioAddress address = {
    .i2c_address = VALID_I2C_ADDRESS,  //
    .pin = INVALID_GPIO_PIN,           //
  };
  Pca9539rGpioState state;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pca9539r_gpio_get_state(&address, &state));
}
void prv_test(const struct GpioAddress *address, void *context) {
  GpioState state = GPIO_STATE_LOW;
  gpio_get_state(address, &state);
}
void test_pca9539r_gpio_subscribe_interrupts(void) {
  GpioAddress address = { .pin = VALID_PORT_0_PIN, .port = 0 };
  TEST_ASSERT_OK(pca9539r_gpio_subscribe_interrupts(&address, &prv_test, NULL));
}

void test_pca9539r_gpio_subscribe_interrupts_invalid(void) {
  GpioAddress invalid_address = { .pin = INVALID_GPIO_PIN, .port = 1 };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    pca9539r_gpio_subscribe_interrupts(&invalid_address, NULL, NULL));
}