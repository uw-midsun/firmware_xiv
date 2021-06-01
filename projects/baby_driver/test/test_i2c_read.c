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
#include "i2c_read.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

typedef struct I2CReadCommand {
  I2CPort port;
  I2CAddress address;
  uint8_t rx_len;
  uint8_t is_reg;
  uint8_t reg;
} I2CReadCommand;

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
static uint8_t s_source[266] = { 0 };
static uint8_t s_dest[266] = { 0 };
static size_t s_bytes;

StatusCode TEST_MOCK(i2c_read)(I2CPort i2c, I2CAddress address, uint8_t *rx_data, size_t rx_len) {
  s_test_storage.port = i2c;
  s_test_storage.address = address;
  // Make register 0 to indicate it is not a register read
  s_test_storage.reg = 0;
  s_test_storage.rx_len = rx_len;
  memcpy(rx_data, s_source, rx_len);
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(i2c_read_reg)(I2CPort i2c, I2CAddress address, uint8_t reg, uint8_t *rx_data,
                                   size_t rx_len) {
  s_test_storage.port = i2c;
  s_test_storage.address = address;
  s_test_storage.reg = reg;
  s_test_storage.rx_len = rx_len;
  memcpy(rx_data, s_source, rx_len);
  return STATUS_CODE_OK;
}

static StatusCode prv_rx_i2c_read_callback(uint8_t data[8], void *context, bool *tx_result) {
  for (size_t i = 1; i < 8; i++) {
    s_dest[s_bytes++] = data[i];
  }
  *tx_result = false;

  return STATUS_CODE_OK;
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BABYDRIVER, TEST_CAN_EVENT_TX,
                                  TEST_CAN_EVENT_RX, TEST_CAN_EVENT_FAULT);
  TEST_ASSERT_OK(dispatcher_init());
  dispatcher_register_callback(BABYDRIVER_MESSAGE_I2C_READ_DATA, prv_rx_i2c_read_callback, NULL);
  TEST_ASSERT_OK(i2c_read_init(I2C_READ_DEFAULT_TX_DELAY_MS));

  s_received_status = 0;
  memset(s_source, 0, sizeof(s_source));
  memset(s_dest, 0, sizeof(s_dest));
  s_bytes = 0;
}

void test_read_i2c(void) {
  // Initial Test
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

  for (uint8_t i = 0; i < data_rx_len; i++) {
    s_source[i] = 1;
  }

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

  // Test to see if the data that is read was sent over CAN successfully
  TEST_ASSERT_EQUAL_UINT8_ARRAY(s_source, s_dest, 255);
}

void test_multiple_msg(void) {
  // Test receiving multiple messages and rx_len not divisible
  // by 7
  I2CPort data_port = I2C_PORT_1;
  I2CAddress data_address = 0x55;
  uint8_t data_rx_len = 13;
  uint8_t data_is_reg = 0;
  uint8_t data_reg = 2;
  uint8_t data[8] = { BABYDRIVER_MESSAGE_I2C_READ_COMMAND,
                      data_port,
                      data_address,
                      data_rx_len,
                      data_is_reg,
                      data_reg,
                      0,
                      0 };

  for (uint8_t i = 0; i < data_rx_len; i++) {
    s_source[i] = 2;
  }

  // Send CAN message with data message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Compares data received to data sent
  TEST_ASSERT_EQUAL(data_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(data_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(0, s_test_storage.reg);
  TEST_ASSERT_EQUAL(data_rx_len, s_test_storage.rx_len);

  // Test to see if the data that is read was sent over CAN successfully
  TEST_ASSERT_EQUAL_UINT8_ARRAY(s_source, s_dest, 255);
}

void test_multiple_msg_port_2(void) {
  // Test receiving multiple messages and rx_len not divisible
  // by 7 on I2C_PORT_2
  I2CPort data_port = I2C_PORT_2;
  I2CAddress data_address = 0x55;
  uint8_t data_rx_len = 13;
  uint8_t data_is_reg = 0;
  uint8_t data_reg = 2;
  uint8_t data[8] = { BABYDRIVER_MESSAGE_I2C_READ_COMMAND,
                      data_port,
                      data_address,
                      data_rx_len,
                      data_is_reg,
                      data_reg,
                      0,
                      0 };

  for (uint8_t i = 0; i < data_rx_len; i++) {
    s_source[i] = 3;
  }

  // Send CAN message with data message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Compares data received to data sent
  TEST_ASSERT_EQUAL(data_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(data_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(0, s_test_storage.reg);
  TEST_ASSERT_EQUAL(data_rx_len, s_test_storage.rx_len);

  // Test to see if the data that is read was sent over CAN successfully
  TEST_ASSERT_EQUAL_UINT8_ARRAY(s_source, s_dest, 255);
}

void test_max_rx_len(void) {
  // Test receiving 255 messages
  I2CPort data_port = I2C_PORT_2;
  I2CAddress data_address = 0x55;
  uint8_t data_rx_len = 255;
  uint8_t data_is_reg = 0;
  uint8_t data_reg = 2;
  uint8_t data[8] = { BABYDRIVER_MESSAGE_I2C_READ_COMMAND,
                      data_port,
                      data_address,
                      data_rx_len,
                      data_is_reg,
                      data_reg,
                      0,
                      0 };

  for (uint8_t i = 0; i < data_rx_len; i++) {
    s_source[i] = 4;
  }

  // Send CAN message with data message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  for (uint8_t i = 0; i < 39; i++) {
    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  }
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Compares data received with the data sent
  TEST_ASSERT_EQUAL(data_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(data_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(0, s_test_storage.reg);
  TEST_ASSERT_EQUAL(data_rx_len, s_test_storage.rx_len);

  // Test to see if the data that is read was sent over CAN successfully
  TEST_ASSERT_EQUAL_UINT8_ARRAY(s_source, s_dest, 255);
}

void test_max_rx_len_reg(void) {
  // Test max length of rx_len with register read
  I2CPort data_port = I2C_PORT_2;
  I2CAddress data_address = 0x55;
  uint8_t data_rx_len = 255;
  uint8_t data_is_reg = 1;
  uint8_t data_reg = 2;
  uint8_t data[8] = { BABYDRIVER_MESSAGE_I2C_READ_COMMAND,
                      data_port,
                      data_address,
                      data_rx_len,
                      data_is_reg,
                      data_reg,
                      0,
                      0 };

  for (uint8_t i = 0; i < data_rx_len; i++) {
    s_source[i] = 5;
  }

  // Send CAN message with data message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  for (int i = 0; i < 39; i++) {
    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  }
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Compares data received with the data sent
  TEST_ASSERT_EQUAL(data_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(data_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(2, s_test_storage.reg);
  TEST_ASSERT_EQUAL(data_rx_len, s_test_storage.rx_len);

  // Test to see if the data that is read was sent over CAN successfully
  TEST_ASSERT_EQUAL_UINT8_ARRAY(s_source, s_dest, 255);
}

void test_less_bytes(void) {
  // Test reading less than 7 bytes
  I2CPort data_port = I2C_PORT_2;
  I2CAddress data_address = 0x55;
  uint8_t data_rx_len = 6;
  uint8_t data_is_reg = 1;
  uint8_t data_reg = 4;
  uint8_t data[8] = { BABYDRIVER_MESSAGE_I2C_READ_COMMAND,
                      data_port,
                      data_address,
                      data_rx_len,
                      data_is_reg,
                      data_reg,
                      0,
                      0 };

  for (uint8_t i = 0; i < data_rx_len; i++) {
    s_source[i] = 6;
  }

  // Send CAN message with command message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Compares received to data sent
  TEST_ASSERT_EQUAL(data_port, s_test_storage.port);
  TEST_ASSERT_EQUAL(data_address, s_test_storage.address);
  TEST_ASSERT_EQUAL(data_reg, s_test_storage.reg);
  TEST_ASSERT_EQUAL(data_rx_len, s_test_storage.rx_len);

  // Test to see if the data that is read was sent over CAN successfully
  TEST_ASSERT_EQUAL_UINT8_ARRAY(s_source, s_dest, 255);
}
