#include "adc.h"
#include "bts_7040_7008_current_sense.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_I2C_PORT I2C_PORT_2
#define TEST_I2C_ADDRESS 0x74

#define TEST_CONFIG_PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define TEST_CONFIG_PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

void setup_test(void) {
  gpio_init();
  interrupt_init();
  adc_init(ADC_MODE_SINGLE);

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,         //
    .sda = TEST_CONFIG_PIN_I2C_SDA,  //
    .scl = TEST_CONFIG_PIN_I2C_SCL,  //
  };
  i2c_init(TEST_I2C_PORT, &i2c_settings);

  pca9539r_gpio_init(TEST_I2C_PORT, TEST_I2C_ADDRESS);
}
void teardown_test(void) {}

// Test that we can initialize successfully with a valid configuration.
void test_bts_7040_init_works(void) {
  Pca9539rGpioAddress test_enable_pin = { .i2c_address = TEST_I2C_ADDRESS,
                                          .pin = PCA9539R_PIN_IO0_0 };
  GpioAddress test_output_pin = { .port = GPIO_PORT_A, .pin = 0 };
  Bts7040Settings valid_settings = {
    .enable_pin = &test_enable_pin,
    .sense_pin = &test_output_pin,
  };
  Bts7040Storage storage;
  TEST_ASSERT_OK(bts_7040_init(&storage, &valid_settings));
}

// Test that it fails with an invalid configuration.
void test_bts_7040_init_invalid_config(void) {
  Pca9539rGpioAddress test_enable_pin = { .i2c_address = TEST_I2C_ADDRESS,
                                          .pin = NUM_PCA9539R_GPIO_PINS };
  GpioAddress test_output_pin = { .port = GPIO_PORT_A, .pin = 0 };
  Bts7040Settings invalid_settings = {
    .enable_pin = &test_enable_pin,
    .sense_pin = &test_output_pin,
  };
  Bts7040Storage storage;

  TEST_ASSERT_NOT_OK(bts_7040_init(&storage, &invalid_settings));
  test_enable_pin.pin = PCA9539R_PIN_IO0_0;
  test_output_pin.port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(bts_7040_init(&storage, &invalid_settings));
}

// Test that we can successfully get a value.
void test_bts_7040_get_measurement(void) {
  Pca9539rGpioAddress test_enable_pin = { .i2c_address = TEST_I2C_ADDRESS,
                                          .pin = PCA9539R_PIN_IO0_0 };
  GpioAddress test_output_pin = { .port = GPIO_PORT_A, .pin = 0 };
  Bts7040Settings settings = {
    .enable_pin = &test_enable_pin,
    .sense_pin = &test_output_pin,
  };
  Bts7040Storage storage;
  TEST_ASSERT_OK(bts_7040_init(&storage, &settings));

  uint16_t measured = 0;
  TEST_ASSERT_OK(bts_7040_get_measurement(&storage, &measured));
  LOG_DEBUG("BTS7040/7008 reading: %d\r\n", measured);
}
