#include "i2c_write.h"

#include <string.h>
#include <time.h>

#include "babydriver_msg_defs.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "delay.h"
#include "dispatcher.h"
#include "gpio.h"
#include "i2c.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_TIMEOUT_PERIOD_MS 50

typedef enum {
  TEST_CAN_EVENT_TX = 0,
  TEST_CAN_EVENT_RX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

// Stores data for mocks of i2c_write and i2c_write_reg
static I2CWriteCommand s_test_storage = { 0 };
// A maximum of 255 bytes of data can be written over i2c
static uint8_t s_tx_data_test[255];

static CanStorage s_can_storage;
static uint8_t s_times_callback_called;
static uint8_t s_received_status;
static void *s_received_context;
static bool s_timeout_test = false;

StatusCode TEST_MOCK(i2c_write)(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len) {
  s_test_storage.port = i2c;
  s_test_storage.address = addr;
  // Make register 0 to indicate i2c_write was called
  s_test_storage.reg = 0;
  s_test_storage.tx_len = tx_len;
  memcpy(s_tx_data_test, tx_data, 255);
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(i2c_write_reg)(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data,
                                    size_t tx_len) {
  s_test_storage.port = i2c;
  s_test_storage.address = addr;
  s_test_storage.reg = reg;
  s_test_storage.tx_len = tx_len;
  memcpy(s_tx_data_test, tx_data, 255);
  return STATUS_CODE_OK;
}

static StatusCode prv_rx_i2c_write_callback(uint8_t data[8], void *context, bool *tx_result) {
  s_times_callback_called++;
  s_received_status = data[1];
  s_received_context = context;
  *tx_result = false;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BABYDRIVER, TEST_CAN_EVENT_TX,
                                  TEST_CAN_EVENT_RX, TEST_CAN_EVENT_FAULT);
  TEST_ASSERT_OK(dispatcher_init());
  // For the timeout test a 50ms timeout period is passed otherwise the 750ms default timeout is
  // used
  if (s_timeout_test) {
    TEST_ASSERT_OK(i2c_write_init(TEST_TIMEOUT_PERIOD_MS));
  } else {
    TEST_ASSERT_OK(i2c_write_init(I2C_WRITE_DEFAULT_TIMEOUT_MS));
  }
  s_times_callback_called = 0;

  TEST_ASSERT_OK(
      dispatcher_register_callback(BABYDRIVER_MESSAGE_STATUS, prv_rx_i2c_write_callback, NULL));
}

void teardown_test(void) {}

// Test that data is correctly written over I2C
void test_writing_over_i2c(void) {
  I2CPort command_port = I2C_PORT_1;
  I2CAddress command_address = 0x74;
  uint8_t command_tx_len = 14;
  uint8_t command_is_reg = 2;
  uint8_t command_reg = 3;
  uint8_t command[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND,
                         command_port,
                         command_address,
                         command_tx_len,
                         command_is_reg,
                         command_reg,
                         0,
                         0 };

  // Test receiving multiple messages and writing over i2c with given register
  // Uses two data messages with valid parameters
  uint8_t data_1[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_DATA, 10, 22, 5, 4, 3, 76, 120 };
  uint8_t data_2[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_DATA, 43, 2, 3, 4, 5, 1, 8 };

  // Send CAN message with command message information
  CAN_TRANSMIT_BABYDRIVER(command[0], command[1], command[2], command[3], command[4], command[5],
                          command[6], command[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Send CAN message with data to be written over i2c
  CAN_TRANSMIT_BABYDRIVER(data_1[0], data_1[1], data_1[2], data_1[3], data_1[4], data_1[5],
                          data_1[6], data_1[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Send CAN message with data to be written over i2c
  CAN_TRANSMIT_BABYDRIVER(data_2[0], data_2[1], data_2[2], data_2[3], data_2[4], data_2[5],
                          data_2[6], data_2[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_status);

  // Compares data written over i2c to values sent over the command and data messages
  TEST_ASSERT_EQUAL(command_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(command_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(command_reg, s_test_storage.reg);
  TEST_ASSERT_EQUAL(command_tx_len, s_test_storage.tx_len);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data_1 + 1, s_tx_data_test, 7);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data_2 + 1, s_tx_data_test + 7, 7);

  // Test receiving multiple messages and writing over i2c without register and tx_len not divisible
  // by 7
  command_port = I2C_PORT_2;
  command_address = 0x55;
  command_tx_len = 13;
  // Register 4 is specified but is_reg indicates that it is a regular write
  command_is_reg = 0;
  command_reg = 4;
  command[1] = command_port;
  command[2] = command_address;
  command[3] = command_tx_len;
  command[4] = command_is_reg;
  command[5] = command_reg;

  // Send CAN message with command message information
  CAN_TRANSMIT_BABYDRIVER(command[0], command[1], command[2], command[3], command[4], command[5],
                          command[6], command[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Send CAN message with data to be written over i2c
  CAN_TRANSMIT_BABYDRIVER(data_1[0], data_1[1], data_1[2], data_1[3], data_1[4], data_1[5],
                          data_1[6], data_1[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Send CAN message with data to be written over i2c
  CAN_TRANSMIT_BABYDRIVER(data_2[0], data_2[1], data_2[2], data_2[3], data_2[4], data_2[5],
                          data_2[6], data_2[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_times_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_status);

  // Compares data written over i2c to values sent over the command and data messages
  TEST_ASSERT_EQUAL(command_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(command_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(0, s_test_storage.reg);
  TEST_ASSERT_EQUAL(command_tx_len, s_test_storage.tx_len);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data_1 + 1, s_tx_data_test, 7);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data_2 + 1, s_tx_data_test + 7, 6);

  // Test receiving only one message and writing exactly 7 bytes of data over I2C
  command_tx_len = 7;
  command[3] = command_tx_len;

  // Send CAN message with command message information
  CAN_TRANSMIT_BABYDRIVER(command[0], command[1], command[2], command[3], command[4], command[5],
                          command[6], command[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Send CAN message with data to be written over i2c
  CAN_TRANSMIT_BABYDRIVER(data_1[0], data_1[1], data_1[2], data_1[3], data_1[4], data_1[5],
                          data_1[6], data_1[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(3, s_times_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_status);

  // Compares data written over i2c to values sent over the command and data messages
  TEST_ASSERT_EQUAL(command_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(command_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(0, s_test_storage.reg);
  TEST_ASSERT_EQUAL(command_tx_len, s_test_storage.tx_len);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data_1 + 1, s_tx_data_test, 7);

  // Test receving max messages(37) and writing the maximum of 255 bytes of data over I2C
  command_tx_len = 255;
  command[3] = command_tx_len;

  // Send CAN message with command message information
  CAN_TRANSMIT_BABYDRIVER(command[0], command[1], command[2], command[3], command[4], command[5],
                          command[6], command[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Sends the first 252 bytes of data to be written over i2c
  for (uint8_t i = 0; i < 36; i++) {
    CAN_TRANSMIT_BABYDRIVER(data_1[0], data_1[1], data_1[2], data_1[3], data_1[4], data_1[5],
                            data_1[6], data_1[7]);
    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  }
  // Sends the last 3 bytes of data
  CAN_TRANSMIT_BABYDRIVER(data_1[0], data_1[1], data_1[2], data_1[3], data_1[4], data_1[5],
                          data_1[6], data_1[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(4, s_times_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_status);

  // Compares data written over i2c to values sent over the command and data messages
  TEST_ASSERT_EQUAL(command_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(command_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(0, s_test_storage.reg);
  TEST_ASSERT_EQUAL(command_tx_len, s_test_storage.tx_len);
  for (uint8_t j = 0; j < 36; j++) {
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data_1 + 1, s_tx_data_test + 7 * j, 7);
  }
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data_1 + 1, s_tx_data_test + 252, 3);

  // Test writing less than 7 bytes of data over i2c
  command_tx_len = 6;
  command_is_reg = 1;
  command_reg = 4;
  command[3] = command_tx_len;
  command[4] = command_is_reg;
  command[5] = command_reg;

  // Send CAN message with command message information
  CAN_TRANSMIT_BABYDRIVER(command[0], command[1], command[2], command[3], command[4], command[5],
                          command[6], command[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Send CAN message with data to be written over i2c
  CAN_TRANSMIT_BABYDRIVER(data_1[0], data_1[1], data_1[2], data_1[3], data_1[4], data_1[5],
                          data_1[6], data_1[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(5, s_times_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_status);

  // Compares data written over i2c to values sent over the command and data messages
  TEST_ASSERT_EQUAL(command_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(command_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(command_reg, s_test_storage.reg);
  TEST_ASSERT_EQUAL(command_tx_len, s_test_storage.tx_len);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data_1 + 1, s_tx_data_test, 6);
}

// Test graceful failure when invalid arguments are sent
void test_invalid_args_i2c(void) {
  I2CPort command_port = I2C_PORT_1;
  I2CAddress command_address = 10;
  uint8_t command_tx_len = 7;
  uint8_t command_is_reg = 0;
  uint8_t command_reg = 0;
  uint8_t command[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND,
                         command_port,
                         command_address,
                         command_tx_len,
                         command_is_reg,
                         command_reg,
                         0,
                         0 };

  // Test data message being received before command message
  // Sends CAN message with command message information with different babydriver_id
  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_I2C_WRITE_DATA, command[1], command[2], command[3],
                          command[4], command[5], command[6], command[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_status);

  // Test command message received when a data callback is expected
  command_tx_len = 8;
  command[3] = command_tx_len;

  uint8_t data[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_DATA, 0, 0, 0, 0, 0, 0, 0 };

  // Send CAN message with command message information
  CAN_TRANSMIT_BABYDRIVER(command[0], command[1], command[2], command[3], command[4], command[5],
                          command[6], command[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Send CAN message with data to be written over i2c
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Send CAN message with babydriver_id for the command callback instead of data callback
  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND, data[1], data[2], data[3], data[4],
                          data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_times_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_status);

  // Test invalid port for command message
  command_port = 2;
  command[1] = command_port;

  // Send CAN message with command message information
  CAN_TRANSMIT_BABYDRIVER(command[0], command[1], command[2], command[3], command[4], command[5],
                          command[6], command[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(3, s_times_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_status);

  // Setup i2c_write_init() with shorter timeout to speed up tests
  s_timeout_test = true;
}

// Test whether timeout error is called when no message is returned
void test_timeout_error_i2c(void) {
  I2CPort command_port = I2C_PORT_1;
  I2CAddress command_address = 0x74;
  uint8_t command_tx_len = 8;
  uint8_t command_is_reg = 2;
  uint8_t command_reg = 3;
  uint8_t command[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND,
                         command_port,
                         command_address,
                         command_tx_len,
                         command_is_reg,
                         command_reg,
                         0,
                         0 };

  // Test delay between 1st and 2nd data message
  uint8_t data[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_DATA, 10, 22, 5, 4, 3, 76, 120 };

  // Send CAN message with command message information
  CAN_TRANSMIT_BABYDRIVER(command[0], command[1], command[2], command[3], command[4], command[5],
                          command[6], command[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Send CAN message with data to be written over i2c
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Delay before next message to trigger timeout error
  delay_ms(TEST_TIMEOUT_PERIOD_MS);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, s_received_status);
}
