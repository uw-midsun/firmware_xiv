#include "i2c_read.h"

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
#define I2C_READ_DEFAULT_TIMEOUT_MS 1500
#define I2C_READ_SOFT_TIMER_TIMEOUT_MS 5

typedef enum {
  TEST_CAN_EVENT_TX = 0,
  TEST_CAN_EVENT_RX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

// Stores data of i2c_read and i2c_read_reg
static I2CReadCommand s_test_storage = { 0 };

static CanStorage s_can_storage;
static uint8_t s_received_status;
static uint8_t s_rx_data_test[255] = { 0 };
static size_t s_bytes;

StatusCode TEST_MOCK(i2c_read)(I2CPort i2c, I2CAddress address, uint8_t *rx_data, size_t rx_len) {
  s_test_storage.port = i2c;
  s_test_storage.address = address;
  // Make register 0 to indicate it is not a register read
  s_test_storage.reg = 0;
  s_test_storage.rx_len = rx_len;
  memcpy(s_rx_data_test, rx_data, 255);
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(i2c_read_reg)(I2CPort i2c, I2CAddress address, uint8_t reg, uint8_t *rx_data,
                                   size_t rx_len) {
  s_test_storage.port = i2c;
  s_test_storage.address = address;
  s_test_storage.reg = reg;
  s_test_storage.rx_len = rx_len;
  memcpy(s_rx_data_test, rx_data, 255);
  return STATUS_CODE_OK;
}

static StatusCode prv_rx_i2c_read_callback(uint8_t data[8], void *context, bool *tx_result) {
  for (size_t i = 0; i < 8; i++) {
    s_rx_data_test[s_bytes] = data[i];
    if (data[i] != 0) s_bytes++;
  }
  *tx_result = false;

  return STATUS_CODE_OK;
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BABYDRIVER, TEST_CAN_EVENT_TX,
                                  TEST_CAN_EVENT_RX, TEST_CAN_EVENT_FAULT);
  TEST_ASSERT_OK(dispatcher_init());
  dispatcher_register_callback(BABYDRIVER_MESSAGE_I2C_READ_DATA, prv_rx_i2c_read_callback, NULL);
  i2c_read_init(I2C_READ_DEFAULT_TIMEOUT_MS, I2C_READ_SOFT_TIMER_TIMEOUT_MS);
}

void test_read_i2c(void) {
  I2CPort data_port = I2C_PORT_1;
  I2CAddress data_address = 0x74;
  uint8_t data_rx_len = 14;
  uint8_t data_is_reg = 1;
  uint8_t data_reg = 3;
  uint8_t data[8] = { BABYDRIVER_MESSAGE_I2C_READ_COMMAND,
                      data_port,
                      data_address,
                      data_rx_len,
                      data_is_reg,
                      data_reg,
                      0,
                      0 };

  // Send CAN message with data
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Compares data from i2c_read or i2c_read_reg to test values
  TEST_ASSERT_EQUAL(data_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(data_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(data_reg, s_test_storage.reg);
  TEST_ASSERT_EQUAL(data_rx_len, s_test_storage.rx_len);

  // Test receiving multiple messages and rx_len not divisible
  // by 7
  data_port = I2C_PORT_1;
  data_address = 0x55;
  data_rx_len = 13;
  data_is_reg = 0;
  data_reg = 2;
  data[1] = data_port;
  data[2] = data_address;
  data[3] = data_rx_len;
  data[4] = data_is_reg;
  data[5] = data_reg;

  // Send CAN message with data message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Compares data recieved to data sent
  TEST_ASSERT_EQUAL(data_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(data_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(0, s_test_storage.reg);
  TEST_ASSERT_EQUAL(data_rx_len, s_test_storage.rx_len);

  // Test receiving multiple messages and rx_len not divisible
  // by 7 on I2C_PORT_2
  data_port = I2C_PORT_2;
  data_address = 0x55;
  data_rx_len = 13;
  data_is_reg = 0;
  data_reg = 2;
  data[1] = data_port;
  data[2] = data_address;
  data[3] = data_rx_len;
  data[4] = data_is_reg;
  data[5] = data_reg;

  // Send CAN message with data message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Compares data recieved to data sent
  TEST_ASSERT_EQUAL(data_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(data_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(0, s_test_storage.reg);
  TEST_ASSERT_EQUAL(data_rx_len, s_test_storage.rx_len);

  // Test max length of rx_len
  data_rx_len = 255;
  data[3] = data_rx_len;

  // Send CAN message with data message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  for (int i = 0; i < 39; i++) {
    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  }
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Compares data recieved with the data sent
  TEST_ASSERT_EQUAL(data_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(data_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(0, s_test_storage.reg);
  TEST_ASSERT_EQUAL(data_rx_len, s_test_storage.rx_len);

  // Test max length of rx_len with register read
  data_rx_len = 255;
  data_is_reg = 1;
  data[3] = data_rx_len;
  data[4] = data_is_reg;

  // Send CAN message with data message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  for (int i = 0; i < 39; i++) {
    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  }
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Compares data recieved with the data sent
  TEST_ASSERT_EQUAL(data_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(data_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(0, s_test_storage.reg);
  TEST_ASSERT_EQUAL(data_rx_len, s_test_storage.rx_len);

  // Test reading less than 7 bytes
  data_rx_len = 6;
  data_is_reg = 1;
  data_reg = 4;
  data[3] = data_rx_len;
  data[4] = data_is_reg;
  data[5] = data_reg;

  // Send CAN message with command message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Compares recieved to data sent
  TEST_ASSERT_EQUAL(data_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(data_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(data_reg, s_test_storage.reg);
  TEST_ASSERT_EQUAL(data_rx_len, s_test_storage.rx_len);
}

void test_timeout_error(void) {
  // This test tests the watchdog timeout error in i2c_read

  I2CPort data_port = I2C_PORT_1;
  I2CAddress data_address = 0x74;
  uint8_t data_rx_len = 8;
  uint8_t data_is_reg = 2;
  uint8_t data_reg = 3;
  uint8_t data[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND,
                      data_port,
                      data_address,
                      data_rx_len,
                      data_is_reg,
                      data_reg,
                      0,
                      0 };

  // Test delay between 1st and 2nd data message
  uint8_t data_2[8] = { BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND, I2C_PORT_2, 0x74, 2, 4, 3, 0, 0 };

  // Send CAN message with command message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Delay before next message to trigger timeout error
  delay_ms(TEST_TIMEOUT_PERIOD_MS);
  CAN_TRANSMIT_BABYDRIVER(data_2[0], data_2[1], data_2[2], data_2[3], data_2[4], data_2[5],
                          data_2[6], data_2[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, s_received_status);
}
