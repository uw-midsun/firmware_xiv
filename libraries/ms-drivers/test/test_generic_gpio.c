#include "controller_board_pins.h"
#include "generic_gpio.h"
#include "log.h"
#include "ms_test_helpers.h"

#define TEST_I2C_PORT I2C_PORT_2
#define TEST_I2C_ADDRESS 0x74

// For testing gpio_set_state
static GpioAddress s_test_gpio_set_addr;
static GpioState s_test_gpio_set_state;

StatusCode TEST_MOCK(gpio_set_state)(const GpioAddress *address, GpioState state) {
  LOG_DEBUG("gpio_set_state called\n");
  s_test_gpio_set_addr = *address;
  s_test_gpio_set_state = state;
  return STATUS_CODE_OK;
}

// For testing gpio_get_state
static GpioState s_test_gpio_get_state;

StatusCode TEST_MOCK(gpio_get_state)(const GpioAddress *address, GpioState *input_state) {
  LOG_DEBUG("gpio_get_state called\n");
  *input_state = s_test_gpio_get_state;
  return STATUS_CODE_OK;
}

// For testing pca9539r_gpio_set_state
static Pca9539rGpioAddress s_test_pca_set_addr;
static Pca9539rGpioState s_test_pca_set_state;

StatusCode TEST_MOCK(pca9539r_gpio_set_state)(const Pca9539rGpioAddress *address,
                                              Pca9539rGpioState input_state) {
  LOG_DEBUG("pca9539r_set_state called\n");
  s_test_pca_set_addr = *address;
  s_test_pca_set_state = input_state;
  return STATUS_CODE_OK;
}

// For testing pca9539r_gpio_get_state
static Pca9539rGpioState s_test_pca_get_state;

StatusCode TEST_MOCK(pca9539r_gpio_get_state)(const Pca9539rGpioAddress *address,
                                              Pca9539rGpioState *input_state) {
  LOG_DEBUG("pca9539r_gpio_get_state called\n");
  *input_state = s_test_pca_get_state;
  return STATUS_CODE_OK;
}

// For testing mcp23008_gpio_set_state
static Mcp23008GpioAddress s_test_mcp_set_addr;
static Mcp23008GpioState s_test_mcp_set_state;

StatusCode TEST_MOCK(mcp23008_gpio_set_state)(const Mcp23008GpioAddress *address,
                                              const Mcp23008GpioState state) {
  LOG_DEBUG("mcp23008_gpio_set_state called\n");
  s_test_mcp_set_addr = *address;
  s_test_mcp_set_state = state;
  return STATUS_CODE_OK;
}

// For testing mcp23008_gpio_get_state
static Mcp23008GpioState s_test_mcp_get_state;

StatusCode TEST_MOCK(mcp23008_gpio_get_state)(const Mcp23008GpioAddress *address,
                                              Mcp23008GpioState *input_state) {
  LOG_DEBUG("mcp23008_gpio_get_state called\n");
  *input_state = s_test_mcp_get_state;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,                //
    .sda = CONTROLLER_BOARD_ADDR_I2C2_SDA,  //
    .scl = CONTROLLER_BOARD_ADDR_I2C2_SCL,  //
  };
  i2c_init(TEST_I2C_PORT, &i2c_settings);

  pca9539r_gpio_init(TEST_I2C_PORT, TEST_I2C_ADDRESS);

  mcp23008_gpio_init(TEST_I2C_PORT, TEST_I2C_ADDRESS);
}

void teardown_test(void) {}

// Make sure gpio pins initialize OK.
void test_gpio_init(void) {
  GpioAddress test_addr = {
    .port = GPIO_PORT_A,
    .pin = 0,
  };
  GpioSettings test_settings = { 0 };
  GenGpioAddress gen_addr = { 0 };

  TEST_ASSERT_OK(generic_gpio_init_gpio_pin(&test_addr, &test_settings, &gen_addr));
}

// Initialize and set pin to high/low.  Make sure calls get routed correctly.
void test_gpio_general_operation(void) {
  GpioAddress test_addr = {
    .port = GPIO_PORT_A,
    .pin = 0,
  };
  GpioSettings test_settings = { 0 };
  GenGpioAddress gen_addr = { 0 };

  TEST_ASSERT_OK(generic_gpio_init_gpio_pin(&test_addr, &test_settings, &gen_addr));

  // Set state
  TEST_ASSERT_OK(generic_gpio_set_state(&gen_addr, GEN_GPIO_STATE_HIGH));
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_gpio_set_state);
  TEST_ASSERT_EQUAL(test_addr.port, s_test_gpio_set_addr.port);
  TEST_ASSERT_EQUAL(test_addr.pin, s_test_gpio_set_addr.pin);

  TEST_ASSERT_OK(generic_gpio_set_state(&gen_addr, GEN_GPIO_STATE_LOW));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_test_gpio_set_state);
  TEST_ASSERT_EQUAL(test_addr.port, s_test_gpio_set_addr.port);
  TEST_ASSERT_EQUAL(test_addr.pin, s_test_gpio_set_addr.pin);

  // Get state
  s_test_gpio_get_state = GPIO_STATE_HIGH;
  GenGpioState test_state = GEN_GPIO_STATE_LOW;
  TEST_ASSERT_OK(generic_gpio_get_state(&gen_addr, &test_state));
  TEST_ASSERT_EQUAL(test_state, GEN_GPIO_STATE_HIGH);
}

// Make sure PCA9539R pins initialize OK.
void test_pca_init(void) {
  Pca9539rGpioSettings test_settings = {
    .direction = PCA9539R_GPIO_DIR_OUT,
    .state = PCA9539R_GPIO_STATE_LOW,
  };

  Pca9539rGpioAddress test_addr = {
    .i2c_address = TEST_I2C_ADDRESS,
    .pin = PCA9539R_PIN_IO0_0,
  };

  GenGpioAddress gen_addr = { 0 };

  TEST_ASSERT_OK(generic_gpio_init_pca9539r(&test_addr, &test_settings, &gen_addr));
}

// Initialize and set pin to high/low for pca9539r.  Make sure calls get routed correctly.
void test_pca_general_operation(void) {
  Pca9539rGpioSettings test_settings = {
    .direction = PCA9539R_GPIO_DIR_OUT,
    .state = PCA9539R_GPIO_STATE_LOW,
  };

  Pca9539rGpioAddress test_addr = {
    .i2c_address = TEST_I2C_ADDRESS,
    .pin = PCA9539R_PIN_IO0_0,
  };

  GenGpioAddress gen_addr = { 0 };

  TEST_ASSERT_OK(generic_gpio_init_pca9539r(&test_addr, &test_settings, &gen_addr));

  // Set state
  TEST_ASSERT_OK(generic_gpio_set_state(&gen_addr, GEN_GPIO_STATE_HIGH));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_HIGH, s_test_pca_set_state);
  TEST_ASSERT_EQUAL(test_addr.i2c_address, s_test_pca_set_addr.i2c_address);
  TEST_ASSERT_EQUAL(test_addr.pin, s_test_pca_set_addr.pin);

  TEST_ASSERT_OK(generic_gpio_set_state(&gen_addr, GEN_GPIO_STATE_LOW));
  TEST_ASSERT_EQUAL(PCA9539R_GPIO_STATE_LOW, s_test_pca_set_state);
  TEST_ASSERT_EQUAL(test_addr.i2c_address, s_test_pca_set_addr.i2c_address);
  TEST_ASSERT_EQUAL(test_addr.pin, s_test_pca_set_addr.pin);

  // Get state
  s_test_pca_get_state = PCA9539R_GPIO_STATE_HIGH;
  GenGpioState test_state = GEN_GPIO_STATE_LOW;
  TEST_ASSERT_OK(generic_gpio_get_state(&gen_addr, &test_state));
  TEST_ASSERT_EQUAL(GEN_GPIO_STATE_HIGH, test_state);
}

// Make sure MCP23008 pins initialize OK.
void test_mcp_init(void) {
  Mcp23008GpioSettings test_settings = {
    .direction = MCP23008_GPIO_DIR_OUT,
    .state = MCP23008_GPIO_STATE_LOW,
  };

  Mcp23008GpioAddress test_addr = {
    .i2c_address = TEST_I2C_ADDRESS,
    .pin = 0,
  };

  GenGpioAddress gen_addr = { 0 };

  TEST_ASSERT_OK(generic_gpio_init_mcp23008(&test_addr, &test_settings, &gen_addr));
}

// Initialize and set pin to high/low for MCP23008.  Make sure calls get routed correctly.
void test_mcp_general_operation(void) {
  Mcp23008GpioSettings test_settings = {
    .direction = MCP23008_GPIO_DIR_OUT,
    .state = MCP23008_GPIO_STATE_LOW,
  };

  Mcp23008GpioAddress test_addr = {
    .i2c_address = TEST_I2C_ADDRESS,
    .pin = 0,
  };

  GenGpioAddress gen_addr = { 0 };

  TEST_ASSERT_OK(generic_gpio_init_mcp23008(&test_addr, &test_settings, &gen_addr));

  // Set state
  TEST_ASSERT_OK(generic_gpio_set_state(&gen_addr, GEN_GPIO_STATE_HIGH));
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_HIGH, s_test_mcp_set_state);
  TEST_ASSERT_EQUAL(test_addr.i2c_address, s_test_mcp_set_addr.i2c_address);
  TEST_ASSERT_EQUAL(test_addr.pin, s_test_mcp_set_addr.pin);

  TEST_ASSERT_OK(generic_gpio_set_state(&gen_addr, GEN_GPIO_STATE_LOW));
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_LOW, s_test_mcp_set_state);
  TEST_ASSERT_EQUAL(test_addr.i2c_address, s_test_mcp_set_addr.i2c_address);
  TEST_ASSERT_EQUAL(test_addr.pin, s_test_mcp_set_addr.pin);

  // Get state
  s_test_mcp_get_state = MCP23008_GPIO_STATE_HIGH;
  GenGpioState test_state = GEN_GPIO_STATE_LOW;
  TEST_ASSERT_OK(generic_gpio_get_state(&gen_addr, &test_state));
  TEST_ASSERT_EQUAL(GEN_GPIO_STATE_HIGH, test_state);
}
