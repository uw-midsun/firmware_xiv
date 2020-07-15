#include "adt7476a_fan_controller.h"
#include "adt7476a_fan_controller_defs.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

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

static Adt7476aMockRegisters s_mock_registers;
static Adt7476aStorage s_mock_storage;
static Adt7476aStorage s_storage;

StatusCode TEST_MOCK(i2c_write_reg)(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data,
                                    size_t tx_len) {
  uint8_t cmd = reg;

  switch (cmd) {
    // Commands used in adt7476a_init()
    case ADT7476A_FAN_MODE_REGISTER_1:
      s_mock_registers.PWM_CONFIG_1 = *tx_data;
      break;
    case ADT7476A_FAN_MODE_REGISTER_3:
      s_mock_registers.PWM_CONFIG_2 = *tx_data;
      break;
    case ADT7476A_CONFIG_REGISTER_3:
      s_mock_registers.SMBALERT_PIN = *tx_data;
      break;
    case ADT7476A_PWM_1:
      s_mock_registers.PWM_SPEED_1 = *tx_data;
      break;
    case ADT7476A_PWM_3:
      s_mock_registers.PWM_SPEED_2 = *tx_data;
      break;
  }
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(i2c_read_reg)(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data,
                                   size_t rx_len) {
  uint8_t cmd = reg;

  switch (cmd) {
    // Commands used in adt7476a_init()
    case ADT7476A_INTERRUPT_STATUS_REGISTER_1:
      *rx_data = s_mock_registers.INTERRUPT_STATUS_1;
      break;
    case ADT7476A_INTERRUPT_STATUS_REGISTER_2:
      *rx_data = s_mock_registers.INTERRUPT_STATUS_2;
      break;
  }
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
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

  s_mock_storage.smbalert_pin = valid_settings.smbalert_pin;
  s_mock_storage.callback = valid_settings.callback;
  s_mock_storage.callback_context = valid_settings.callback_context;
  s_mock_storage.i2c = valid_settings.i2c;

  TEST_ASSERT_OK(adt7476a_init(&s_storage, &valid_settings));

  TEST_ASSERT_EQUAL(s_mock_storage.smbalert_pin.pin, s_storage.smbalert_pin.pin);
  TEST_ASSERT_EQUAL(s_mock_storage.smbalert_pin.port, s_storage.smbalert_pin.port);
  TEST_ASSERT_EQUAL(s_mock_storage.callback, s_storage.callback);
  TEST_ASSERT_EQUAL(s_mock_storage.callback_context, s_storage.callback_context);
  TEST_ASSERT_EQUAL(s_mock_storage.i2c, s_storage.i2c);

  TEST_ASSERT_EQUAL(ADT7476A_MANUAL_MODE_MASK, s_mock_registers.PWM_CONFIG_1);
  TEST_ASSERT_EQUAL(ADT7476A_MANUAL_MODE_MASK, s_mock_registers.PWM_CONFIG_2);
  TEST_ASSERT_EQUAL(ADT7476A_CONFIG_REG_3_MASK, s_mock_registers.SMBALERT_PIN);
}

// test that speeds are set correctly
void test_adt7476a_set_speed(void) {
  uint8_t speed_percent = 50;

  adt7476a_set_speed(TEST_I2C_PORT, speed_percent, ADT_FAN_GROUP_1, TEST_I2C_ADDRESS);

  TEST_ASSERT_EQUAL(TEST_FAN_PWM_EXPECTED_SPEED, s_mock_registers.PWM_SPEED_1);

  adt7476a_set_speed(TEST_I2C_PORT, speed_percent, ADT_FAN_GROUP_2, TEST_I2C_ADDRESS);

  TEST_ASSERT_EQUAL(TEST_FAN_PWM_EXPECTED_SPEED, s_mock_registers.PWM_SPEED_2);
}

void test_adt7476a_set_invalid_speed(void) {
  uint8_t invalid_speed_ercent = 101;

  TEST_ASSERT_NOT_EQUAL(STATUS_CODE_OK, adt7476a_set_speed(TEST_I2C_PORT, invalid_speed_ercent,
                                                           ADT_FAN_GROUP_1, TEST_I2C_ADDRESS));
}

// test if fetching data from register works
void test_adt7476a_get_status(void) {
  s_mock_registers.INTERRUPT_STATUS_1 = 0x01;
  s_mock_registers.INTERRUPT_STATUS_2 = 0x02;

  uint8_t rx_interrupt_status_reg_1;
  uint8_t rx_interrupt_status_reg_2;

  adt7476a_get_status(TEST_I2C_PORT, TEST_I2C_ADDRESS, &rx_interrupt_status_reg_1,
                      &rx_interrupt_status_reg_2);
  TEST_ASSERT_EQUAL(s_mock_registers.INTERRUPT_STATUS_1, rx_interrupt_status_reg_1);
  TEST_ASSERT_EQUAL(s_mock_registers.INTERRUPT_STATUS_2, rx_interrupt_status_reg_2);
}
