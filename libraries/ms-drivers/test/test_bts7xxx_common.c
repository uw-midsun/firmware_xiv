// Short test sequence for functions used on BTS7xxx pins

#include "bts7xxx_common.h"

#include "controller_board_pins.h"
#include "gpio.h"
#include "i2c.h"
#include "test_helpers.h"

#define TEST_I2C_PORT I2C_PORT_2
#define TEST_I2C_ADDRESS 0x74

void setup_test() {
  gpio_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = CONTROLLER_BOARD_ADDR_I2C2_SDA,
    .scl = CONTROLLER_BOARD_ADDR_I2C2_SCL,
  };

  i2c_init(TEST_I2C_PORT, &i2c_settings);
}

void teardown_test() {}

const GpioAddress test_pin_stm32 = { .port = GPIO_PORT_A, .pin = 0 };

const Pca9539rGpioAddress test_pin_pca9539r = { .i2c_address = 0, .pin = PCA9539R_PIN_IO0_0 };

// Test that initialization works on STM32.
void test_bts7xxx_init_pin_stm32() {
  Bts7xxxEnablePin pin = {
    .enable_pin_stm32 = &test_pin_stm32,
    .pin_type = BTS7XXX_PIN_STM32,
  };
  TEST_ASSERT_OK(bts7xxx_init_pin(&pin));
}

// Same, but for PCA9539R.
void test_bts7xxx_init_pin_pca9539r() {
  Bts7xxxEnablePin pin = {
    .enable_pin_pca9539r = &test_pin_pca9539r,
    .pin_type = BTS7XXX_PIN_PCA9539R,
  };
  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, pin.enable_pin_pca9539r->i2c_address));
  TEST_ASSERT_OK(bts7xxx_init_pin(&pin));
}

// Same two tests as above, but with pin settings that should fail.
void test_bts7xxx_init_pin_fails_bad_settings_stm32() {
  GpioAddress test_invalid_pin = { .port = NUM_GPIO_PORTS, .pin = 0 };
  Bts7xxxEnablePin pin = {
    .enable_pin_stm32 = &test_invalid_pin,
    .pin_type = BTS7XXX_PIN_STM32,
  };
  TEST_ASSERT_NOT_OK(bts7xxx_init_pin(&pin));
  // Change to valid settings
  test_invalid_pin.port = GPIO_PORT_A;
  TEST_ASSERT_OK(bts7xxx_enable_pin(&pin));
}

void test_bts7xxx_init_pin_fails_bad_settings_pca9539r() {
  Pca9539rGpioAddress test_invalid_pin = {
    .i2c_address = 0,
    .pin = NUM_PCA9539R_GPIO_PINS,
  };
  Bts7xxxEnablePin pin = {
    .enable_pin_pca9539r = &test_invalid_pin,
    .pin_type = BTS7XXX_PIN_PCA9539R,
  };
  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, pin.enable_pin_pca9539r->i2c_address));
  TEST_ASSERT_NOT_OK(bts7xxx_init_pin(&pin));
  // Change to valid settings
  test_invalid_pin.pin = PCA9539R_PIN_IO0_0;
  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, pin.enable_pin_pca9539r->i2c_address));
}

// Test that enabling STM32 enable pins works as expected.
void test_bts7xxx_enable_pin_stm32() {
  Bts7xxxEnablePin pin = {
    .enable_pin_stm32 = &test_pin_stm32,
    .pin_type = BTS7XXX_PIN_STM32,
  };

  TEST_ASSERT_OK(bts7xxx_init_pin(&pin));
  // Should init low
  TEST_ASSERT_FALSE(bts7xxx_get_pin_enabled(&pin));
  TEST_ASSERT_OK(bts7xxx_enable_pin(&pin));
  TEST_ASSERT_TRUE(bts7xxx_get_pin_enabled(&pin));

  // Make sure enabling pin that's already enabled doesn't cause issues
  TEST_ASSERT_OK(bts7xxx_enable_pin(&pin));
  TEST_ASSERT_TRUE(bts7xxx_get_pin_enabled(&pin));

  // Same idea, but for disabling pin
  TEST_ASSERT_OK(bts7xxx_disable_pin(&pin));
  TEST_ASSERT_FALSE(bts7xxx_get_pin_enabled(&pin));

  TEST_ASSERT_OK(bts7xxx_disable_pin(&pin));
  TEST_ASSERT_FALSE(bts7xxx_get_pin_enabled(&pin));
}

// Same, but for PCA9539R.
void test_bts7xxx_enable_pin_pca9539r() {
  Bts7xxxEnablePin pin = {
    .enable_pin_pca9539r = &test_pin_pca9539r,
    .pin_type = BTS7XXX_PIN_PCA9539R,
  };
  TEST_ASSERT_OK(pca9539r_gpio_init(TEST_I2C_PORT, pin.enable_pin_pca9539r->i2c_address));
  TEST_ASSERT_OK(bts7xxx_init_pin(&pin));

  TEST_ASSERT_OK(bts7xxx_init_pin(&pin));
  // Should init low
  TEST_ASSERT_FALSE(bts7xxx_get_pin_enabled(&pin));
  TEST_ASSERT_OK(bts7xxx_enable_pin(&pin));
  TEST_ASSERT_TRUE(bts7xxx_get_pin_enabled(&pin));

  // Make sure enabling pin that's already enabled doesn't cause issues
  TEST_ASSERT_OK(bts7xxx_enable_pin(&pin));
  TEST_ASSERT_TRUE(bts7xxx_get_pin_enabled(&pin));

  // Same idea, but for disabling pin
  TEST_ASSERT_OK(bts7xxx_disable_pin(&pin));
  TEST_ASSERT_FALSE(bts7xxx_get_pin_enabled(&pin));

  TEST_ASSERT_OK(bts7xxx_disable_pin(&pin));
  TEST_ASSERT_FALSE(bts7xxx_get_pin_enabled(&pin));
}

void test_bts7xxx_enable_fails_during_fault() {
  Bts7xxxEnablePin pin = {
    .enable_pin_stm32 = &test_pin_stm32,
    .pin_type = BTS7XXX_PIN_STM32,
  };

  TEST_ASSERT_OK(bts7xxx_init_pin(&pin));
  // Simulate fault -- shouldn't be able to enable pin
  pin.fault_in_progress = true;
  TEST_ASSERT_NOT_OK(bts7xxx_enable_pin(&pin));
  TEST_ASSERT_TRUE(pin.fault_in_progress);
  TEST_ASSERT_FALSE(bts7xxx_get_pin_enabled(&pin));

  // Should be able to enable pin after fault clears
  pin.fault_in_progress = false;
  TEST_ASSERT_OK(bts7xxx_enable_pin(&pin));
  TEST_ASSERT_TRUE(bts7xxx_get_pin_enabled(&pin));
}
