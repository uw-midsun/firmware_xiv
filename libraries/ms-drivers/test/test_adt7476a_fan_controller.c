#include "adt7476a_fan_controller.h"
#include "adt7476a_fan_controller_defs.h"
#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"
#include "soft_timer.h"

#define TEST_I2C_PORT I2C_PORT_2
#define TEST_I2C_ADDRESS 0x74
#define TEST_FAN_PWM_SPEED_1 0x03
#define TEST_FAN_PWM_SPEED_2 0x04
#define TEST_FAN_PWM_EXPECTED_SPEED 0x80

#define TEST_CONFIG_PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define TEST_CONFIG_PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

typedef struct Adt7476aMockRegisters {
  uint8_t PWM_CONFIG_1;
  uint8_t PWM_CONFIG_2;
  uint8_t SMBALERT_PIN;
  uint8_t PWM_SPEED_1;
  uint8_t PWM_SPEED_2;
  uint8_t INTERRUPT_STATUS_1;
  uint8_t INTERRUPT_STATUS_2;
} Adt7476aMockRegisters;

Adt7476aMockRegisters MockRegisters;
Adt7476aStorage MockStorage;
Adt7476aStorage storage;

StatusCode TEST_MOCK(i2c_write_reg)(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data,
                                    size_t tx_len) {
  uint8_t cmd = reg;

  switch (cmd) {
    // Commands used in ads1259_init()
    case ADT7476A_FAN_MODE_REGISTER_1:
      MockRegisters.PWM_CONFIG_1 = *tx_data;
      break;
    case ADT7476A_FAN_MODE_REGISTER_3:
      MockRegisters.PWM_CONFIG_2 = *tx_data;
      break;
    case ADT7476A_CONFIG_REGISTER_3:
      MockRegisters.SMBALERT_PIN = *tx_data;
      break;
    case ADT7476A_PWM_1:
      MockRegisters.PWM_SPEED_1 = *tx_data;
      break;
    case ADT7476A_PWM_3:
      MockRegisters.PWM_SPEED_2 = *tx_data;
      break;
  }
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(i2c_read_reg)(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data,
                                   size_t rx_len) {
  uint8_t cmd = reg;

  switch (cmd) {
    // Commands used in ads1259_init()
    case ADT7476A_INTERRUPT_STATUS_REGISTER_1:
      *rx_data = MockRegisters.INTERRUPT_STATUS_1;
      break;
    case ADT7476A_INTERRUPT_STATUS_REGISTER_2:
      *rx_data = MockRegisters.INTERRUPT_STATUS_2;
      break;
  }
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
}

void teardown_test(void) {}

// Test initializing with valid parameters
void test_adt7476a_init_works(void) {
  GpioAddress test_output_pin = { .port = GPIO_PORT_A, .pin = 0 };

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,         //
    .sda = TEST_CONFIG_PIN_I2C_SDA,  //
    .scl = TEST_CONFIG_PIN_I2C_SCL,  //
  };

  Adt7476aSettings valid_settings = {
    .smbalert_pin = test_output_pin,
    .callback = NULL,
    .callback_context = NULL,
    .i2c = TEST_I2C_PORT,
    .i2c_read_addr = TEST_I2C_ADDRESS,
    .i2c_write_addr = TEST_I2C_ADDRESS,
    .i2c_settings = i2c_settings,
  };

  MockStorage.smbalert_pin = valid_settings.smbalert_pin;
  MockStorage.callback = valid_settings.callback;
  MockStorage.callback_context = valid_settings.callback_context;
  MockStorage.i2c = valid_settings.i2c;

  TEST_ASSERT_OK(adt7476a_init(&storage, &valid_settings));

  TEST_ASSERT_EQUAL(MockStorage.smbalert_pin.pin, storage.smbalert_pin.pin);
  TEST_ASSERT_EQUAL(MockStorage.smbalert_pin.port, storage.smbalert_pin.port);
  TEST_ASSERT_EQUAL(MockStorage.callback, storage.callback);
  TEST_ASSERT_EQUAL(MockStorage.callback_context, storage.callback_context);
  TEST_ASSERT_EQUAL(MockStorage.i2c, storage.i2c);

  TEST_ASSERT_EQUAL(ADT7476A_MANUAL_MODE_MASK, MockRegisters.PWM_CONFIG_1);
  TEST_ASSERT_EQUAL(ADT7476A_MANUAL_MODE_MASK, MockRegisters.PWM_CONFIG_2);
  TEST_ASSERT_EQUAL(ADT7476A_CONFIG_REG_3_MASK, MockRegisters.SMBALERT_PIN);
}

// test that speeds are set correctly
void test_adt7476a_set_speed(void) {
  uint8_t SPEED_PERCENT = 50;

  adt7476a_set_speed(TEST_I2C_PORT, SPEED_PERCENT, ADT_FAN_GROUP_1, TEST_I2C_ADDRESS);

  TEST_ASSERT_EQUAL(MockRegisters.PWM_SPEED_1, TEST_FAN_PWM_EXPECTED_SPEED);

  adt7476a_set_speed(TEST_I2C_PORT, SPEED_PERCENT, ADT_FAN_GROUP_2, TEST_I2C_ADDRESS);

  TEST_ASSERT_EQUAL(MockRegisters.PWM_SPEED_2, TEST_FAN_PWM_EXPECTED_SPEED);
}

// test if fetching data from register works
void test_adt7476a_get_status(void) {
  MockRegisters.INTERRUPT_STATUS_1 = 0x01;
  MockRegisters.INTERRUPT_STATUS_2 = 0x02;

  uint8_t rx_interrupt_status_reg_1;
  uint8_t rx_interrupt_status_reg_2;

  adt7476a_get_status(TEST_I2C_PORT, TEST_I2C_ADDRESS, &rx_interrupt_status_reg_1,
                      &rx_interrupt_status_reg_2);
  TEST_ASSERT_EQUAL(MockRegisters.INTERRUPT_STATUS_1, rx_interrupt_status_reg_1);
  TEST_ASSERT_EQUAL(MockRegisters.INTERRUPT_STATUS_2, rx_interrupt_status_reg_2);
}
