#include "i2c_write.h"

#include <string.h>

#include "babydriver_msg_defs.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "dispatcher.h"
#include "gpio.h"
#include "i2c.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

#define I2C1_SDA \
  { .port = GPIO_PORT_B, .pin = 9 }
#define I2C1_SCL \
  { .port = GPIO_PORT_B, .pin = 8 }
#define I2C2_SDA \
  { .port = GPIO_PORT_B, .pin = 11 }
#define I2C2_SCL \
  { .port = GPIO_PORT_B, .pin = 10 }

typedef enum {
  TEST_CAN_EVENT_TX = 0,
  TEST_CAN_EVENT_RX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

static CanStorage s_can_storage;
static uint8_t s_times_callback_called;
static uint8_t s_received_data[8];
static void *s_received_context;

static StatusCode s_status_return;

// Stores values from mock functions for i2c_write and i2c_write_reg
static I2CPort i2c_test;
static I2CAddress addr_test;
static uint8_t reg_test;
static uint8_t *tx_data_test;
static size_t tx_len_test;

StatusCode TEST_MOCK(i2c_write)(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len) {
  i2c_test = i2c;
  addr_test = addr;
  reg_test = 0;
  tx_data_test = tx_data;
  tx_len_test = tx_len;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(i2c_write_reg)(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data,
                                    size_t tx_len) {
  i2c_test = i2c;
  addr_test = addr;
  reg_test = reg;
  tx_data_test = tx_data;
  tx_len_test = tx_len;
  return STATUS_CODE_OK;
}

static StatusCode prv_rx_i2c_write_callback(uint8_t data[8], void *context, bool *tx_result) {
  s_times_callback_called++;
  memcpy(s_received_data, data, 8);
  s_received_context = context;
  *tx_result = false;
  return s_status_return;
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BABYDRIVER, TEST_CAN_EVENT_TX,
                                  TEST_CAN_EVENT_RX, TEST_CAN_EVENT_FAULT);
  TEST_ASSERT_OK(dispatcher_init());
  TEST_ASSERT_OK(i2c_write_init());
  s_times_callback_called = 0;
  memset(s_received_data, 0, sizeof(s_received_data));
  s_status_return = STATUS_CODE_OK;

  TEST_ASSERT_OK(
      dispatcher_register_callback(BABYDRIVER_MESSAGE_STATUS, prv_rx_i2c_write_callback, NULL));
}

void teardown_test(void) {}

// Test that data is correctly written over I2C
void test_writing_over_i2c(void) {
  // Test receiving multiple messages and writing over i2c with given register
  uint8_t command_1[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND, I2C_PORT_1, 0x74, 14, 2, 3, 0, 0 };
  uint8_t data_1[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_DATA, 10, 22, 5, 4, 3, 76, 120 };
  uint8_t data_2[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_DATA, 43, 2, 3, 4, 5, 1, 8 };

  CAN_TRANSMIT_BABYDRIVER(command_1[0], command_1[1], command_1[2], command_1[3], command_1[4],
                          command_1[5], command_1[6], command_1[7]);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  CAN_TRANSMIT_BABYDRIVER(data_1[0], data_1[1], data_1[2], data_1[3], data_1[4], data_1[5],
                          data_1[6], data_1[7]);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  CAN_TRANSMIT_BABYDRIVER(data_2[0], data_2[1], data_2[2], data_2[3], data_2[4], data_2[5],
                          data_2[6], data_2[7]);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(s_status_return, s_received_data[1]);

  TEST_ASSERT_EQUAL(I2C_PORT_1, i2c_test);
  TEST_ASSERT_EQUAL(0x74, addr_test);
  TEST_ASSERT_EQUAL(3, reg_test);
  TEST_ASSERT_EQUAL(14, tx_len_test);
  for (uint8_t i = 0; i < 7; i++) {
    TEST_ASSERT_EQUAL(data_1[i + 1], tx_data_test[i]);
  }
  for (uint8_t j = 0; j < 7; j++) {
    TEST_ASSERT_EQUAL(data_2[j + 1], tx_data_test[j + 7]);
  }

  // Test receiving multiple messages and writing over i2c without register
  uint8_t command_2[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND, I2C_PORT_2, 0x55, 14, 0, 0, 0, 0 };

  CAN_TRANSMIT_BABYDRIVER(command_2[0], command_2[1], command_2[2], command_2[3], command_2[4],
                          command_2[5], command_2[6], command_2[7]);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  CAN_TRANSMIT_BABYDRIVER(data_1[0], data_1[1], data_1[2], data_1[3], data_1[4], data_1[5],
                          data_1[6], data_1[7]);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  CAN_TRANSMIT_BABYDRIVER(data_2[0], data_2[1], data_2[2], data_2[3], data_2[4], data_2[5],
                          data_2[6], data_2[7]);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_times_callback_called);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(s_status_return, s_received_data[1]);

  TEST_ASSERT_EQUAL(I2C_PORT_2, i2c_test);
  TEST_ASSERT_EQUAL(0x55, addr_test);
  TEST_ASSERT_EQUAL(0, reg_test);
  TEST_ASSERT_EQUAL(14, tx_len_test);
  for (uint8_t i = 0; i < 7; i++) {
    TEST_ASSERT_EQUAL(data_1[i + 1], tx_data_test[i]);
  }
  for (uint8_t j = 0; j < 7; j++) {
    TEST_ASSERT_EQUAL(data_2[j + 1], tx_data_test[j + 7]);
  }
}

// Test graceful failure when invalid arguments are sent
void test_invalid_args_i2c(void) {
  s_status_return = STATUS_CODE_INVALID_ARGS;
  uint8_t command[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND, I2C_PORT_1, 10, 7, 0, 0, 0, 0 };

  // Invalid babydriver_id for command message
  uint8_t inv_1[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_DATA, I2C_PORT_2, 0, 0, 0, 0, 0, 0 };

  CAN_TRANSMIT_BABYDRIVER(inv_1[0], inv_1[1], inv_1[2], inv_1[3], inv_1[4], inv_1[5], inv_1[6],
                          inv_1[7]);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(s_status_return, s_received_data[1]);

  // Invalid babydriver_id for data messages
  uint8_t inv_2[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND, 0, 0, 0, 0, 0, 0, 0 };

  CAN_TRANSMIT_BABYDRIVER(command[0], command[1], command[2], command[3], command[4], command[5],
                          command[6], command[7]);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  CAN_TRANSMIT_BABYDRIVER(inv_2[0], inv_2[1], inv_2[2], inv_2[3], inv_2[4], inv_2[5], inv_2[6],
                          inv_2[7]);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_times_callback_called);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(s_status_return, s_received_data[1]);

  // Invalid port for command message
  uint8_t inv_3[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND, 2, 0, 0, 0, 0, 0, 0 };

  CAN_TRANSMIT_BABYDRIVER(inv_3[0], inv_3[1], inv_3[2], inv_3[3], inv_3[4], inv_3[5], inv_3[6],
                          inv_3[7]);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(3, s_times_callback_called);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(s_status_return, s_received_data[1]);

  // Invalid tx_len for command message
  uint8_t inv_4[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND, 0, 0, 6, 0, 0, 0, 0 };

  CAN_TRANSMIT_BABYDRIVER(inv_4[0], inv_4[1], inv_4[2], inv_4[5], inv_4[4], inv_4[5], inv_4[6],
                          inv_4[7]);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(4, s_times_callback_called);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(s_status_return, s_received_data[1]);
}

// Test whether timeout error is called when no message is returned
void test_timeout_error_i2c(void) {
  s_status_return = STATUS_CODE_TIMEOUT;

  // Test whether callback will timeout when no data is received after command message
  uint8_t command[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND, I2C_PORT_1, 10, 7, 0, 0, 0, 0 };

  CAN_TRANSMIT_BABYDRIVER(command[0], command[1], command[2], command[3], command[4], command[5],
                          command[6], command[7]);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(s_status_return, s_received_data[1]);
}
