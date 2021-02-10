#include "spi_exchange.h"

#include <string.h>

#include "babydriver_msg_defs.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "controller_board_pins.h"
#include "dispatcher.h"
#include "gpio_mcu.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "spi.h"
#include "test_helpers.h"
#include "unity.h"

#define INVALID_CS_PORT (NUM_GPIO_PORTS)
#define INVALID_CS_PIN (GPIO_PINS_PER_PORT)

#define INVALID_SPI_PORT (NUM_SPI_PORTS)
#define INVALID_SPI_MODE (NUM_SPI_MODES)

#define TEST_BAUDRATE (6000000)

typedef enum {
  TEST_CAN_EVENT_TX = 0,
  TEST_CAN_EVENT_RX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

static CanStorage s_can_storage;

static uint8_t s_callback_counter_status;
static size_t s_num_bytes;
static uint8_t s_received_data_status[2];
static uint8_t s_received_data[296];

static SpiSettings s_spi_settings;
static SpiPort s_spi_port;

static SpiSettings s_spi_settings_test_1 = { .baudrate = 6000000,
                                             .mode = SPI_MODE_0,
                                             .mosi = CONTROLLER_BOARD_ADDR_SPI1_MOSI,
                                             .miso = CONTROLLER_BOARD_ADDR_SPI1_MISO,
                                             .sclk = CONTROLLER_BOARD_ADDR_SPI1_SCK,
                                             .cs = (GpioAddress){ 0, 0 } };
#define SPI_PORT_TEST_1 (SPI_PORT_1)

static SpiSettings s_spi_settings_test_2 = { .baudrate = 6000000,
                                             .mode = SPI_MODE_3,
                                             .mosi = CONTROLLER_BOARD_ADDR_SPI2_MOSI,
                                             .miso = CONTROLLER_BOARD_ADDR_SPI2_MISO,
                                             .sclk = CONTROLLER_BOARD_ADDR_SPI2_SCK,
                                             .cs = CONTROLLER_BOARD_ADDR_SPI2_NSS };
#define SPI_PORT_TEST_2 (SPI_PORT_2)

static StatusCode prv_callback_spi_exchange_status(uint8_t data[2], void *context,
                                                   bool *tx_result) {
  // This is a dispatcher callback to receive messages from the spi_exchange module
  // This callback receives the babydriver status messages

  memcpy(s_received_data_status, data, 2);
  *tx_result = false;
  s_callback_counter_status++;

  return STATUS_CODE_OK;
}

static StatusCode prv_callback_spi_exchange(uint8_t data[8], void *context, bool *tx_result) {
  // This is a dispatcher callback to receive messages from the spi_exchange module
  // This callback receives the data messages

  for (size_t i = 0; i < 8; i++) {
    s_received_data[s_num_bytes] = data[i];
    if (data[i] != 0) s_num_bytes++;
  }
  *tx_result = false;

  return STATUS_CODE_OK;
}

static void prv_send_meta_data(uint8_t port, uint8_t mode, uint8_t tx_len, uint8_t rx_len,
                               uint8_t cs_port, uint8_t cs_pin, uint8_t use_cs, uint32_t baudrate) {
  // This function only sends the first and second metadata message
  // This is used for valid input tests

  // Example of prv_send_meta_data:
  // prv_send_meta_data(port, mode, tx_len, rx_len, cs_port, cs_pin, use_cs, baudrate);

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_SPI_EXCHANGE_METADATA_1,  // id
                          port, mode,                                  // port, mode
                          tx_len, rx_len,                              // tx_len, rx_len
                          cs_port, cs_pin, use_cs);                    // cs_port, cs_pin, use_cs

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  uint8_t baudrate_1 = (baudrate & 0x000000ff);
  uint8_t baudrate_2 = (baudrate & 0x0000ff00) >> 8;
  uint8_t baudrate_3 = (baudrate & 0x00ff0000) >> 16;
  uint8_t baudrate_4 = (baudrate & 0xff000000) >> 24;
  // The baudrate is placed in little endian format

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_SPI_EXCHANGE_METADATA_2,      // id
                          baudrate_1, baudrate_2, baudrate_3, baudrate_4,  // baudrate
                          0, 0, 0);                                        // 3 * uint8 unused
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
}

static void prv_send_first_meta_data(uint8_t port, uint8_t mode, uint8_t tx_len, uint8_t rx_len,
                                     uint8_t cs_port, uint8_t cs_pin, uint8_t use_cs) {
  // This function only sends the first metadata message
  // This is used for timeout and invalid input tests

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_SPI_EXCHANGE_METADATA_1,  // id
                          port, mode,                                  // port, mode
                          tx_len, rx_len,                              // tx_len, rx_len
                          cs_port, cs_pin, use_cs);                    // cs_port, cs_pin, use_cs
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
}

static void prv_clear_received_data_array(void) {
  // This function clears recieved data array

  for (size_t i = 0; i < s_num_bytes; i++) {
    s_received_data[i] = 0;
  }
}

static void prv_test_equal_multiple_messages(void) {
  // This function compares if the data received is correct
  // It also creates the mock_array to compare against the received data

  uint8_t mock_array[296];
  for (size_t i = 0; i < s_num_bytes; i++) {
    if (i % 8 == 0) {
      mock_array[i] = BABYDRIVER_MESSAGE_SPI_EXCHANGE_RX_DATA;
    } else {
      mock_array[i] = 1;
    }
  }

  TEST_ASSERT_EQUAL_INT8_ARRAY(mock_array, s_received_data, s_num_bytes);
}

static void prv_send_data(uint8_t bytes) {
  // This function sends data messages to spi_exchange
  // that the python would normally send

  uint8_t can_data_array[8] = { 0 };
  for (size_t i = 0; i < bytes; i++) {
    can_data_array[i] = 1;
  }

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_SPI_EXCHANGE_TX_DATA, can_data_array[0],
                          can_data_array[1], can_data_array[2], can_data_array[3],
                          can_data_array[4], can_data_array[5], can_data_array[6]);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BABYDRIVER, TEST_CAN_EVENT_TX,
                                  TEST_CAN_EVENT_RX, TEST_CAN_EVENT_FAULT);
  TEST_ASSERT_OK(dispatcher_init());
  TEST_ASSERT_OK(spi_exchange_init(DEFAULT_SPI_EXCHANGE_TIMEOUT_MS, DEFAULT_SPI_EXCHANGE_TX_DELAY));

  prv_clear_received_data_array();
  memset(s_received_data_status, 0, sizeof(s_received_data_status));
  s_callback_counter_status = 0;
  s_num_bytes = 0;

  TEST_ASSERT_OK(dispatcher_register_callback(BABYDRIVER_MESSAGE_STATUS,
                                              prv_callback_spi_exchange_status, NULL));
  TEST_ASSERT_OK(dispatcher_register_callback(BABYDRIVER_MESSAGE_SPI_EXCHANGE_RX_DATA,
                                              prv_callback_spi_exchange, NULL));
}
void teardown_test(void) {}

void test_valid_input(void) {
  // This test tests the different valid inputs for spi_exchange
  // The only test that was omitted was for a tx_len and rx_len of 255
  // This was because there was an error with the TX and RX helpers

  // first test using the given cs port and pin
  // Please see prv_send_meta_data for information on the parameters
  prv_send_meta_data(0, 0, 7, 7, 0, 0, 1, TEST_BAUDRATE);

  prv_send_data(7);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(8, s_num_bytes);
  prv_test_equal_multiple_messages();

  TEST_ASSERT_EQUAL(SPI_PORT_TEST_1, s_spi_port);
  TEST_ASSERT_EQUAL(s_spi_settings_test_1.baudrate, s_spi_settings.baudrate);
  TEST_ASSERT_EQUAL(s_spi_settings_test_1.mode, s_spi_settings.mode);
  TEST_ASSERT_EQUAL(s_spi_settings_test_1.mosi.port, s_spi_settings.mosi.port);
  TEST_ASSERT_EQUAL(s_spi_settings_test_1.mosi.pin, s_spi_settings.mosi.pin);
  TEST_ASSERT_EQUAL(s_spi_settings_test_1.miso.port, s_spi_settings.miso.port);
  TEST_ASSERT_EQUAL(s_spi_settings_test_1.miso.pin, s_spi_settings.miso.pin);
  TEST_ASSERT_EQUAL(s_spi_settings_test_1.sclk.port, s_spi_settings.sclk.port);
  TEST_ASSERT_EQUAL(s_spi_settings_test_1.sclk.pin, s_spi_settings.sclk.pin);
  TEST_ASSERT_EQUAL(s_spi_settings_test_1.cs.port, s_spi_settings.cs.port);
  TEST_ASSERT_EQUAL(s_spi_settings_test_1.cs.pin, s_spi_settings.cs.pin);

  TEST_ASSERT_EQUAL(1, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_data_status[1]);

  prv_clear_received_data_array();
  s_num_bytes = 0;

  // second test using the default cs port and pin
  prv_send_meta_data(0, 0, 7, 7, 0, 0, 0, TEST_BAUDRATE);

  prv_send_data(7);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(8, s_num_bytes);
  prv_test_equal_multiple_messages();

  TEST_ASSERT_EQUAL(2, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_data_status[1]);

  prv_clear_received_data_array();
  s_num_bytes = 0;

  // third test for tx_len and rx_len edge cases (0)
  prv_send_meta_data(0, 0, 0, 0, 0, 0, 0, TEST_BAUDRATE);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(0, s_num_bytes);

  TEST_ASSERT_EQUAL(3, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_data_status[1]);

  prv_clear_received_data_array();
  s_num_bytes = 0;

  // fourth test for tx_len and rx_len edge cases (255)

  // This test was removed because of an error with the MS_TEST_HELPERs
  // Since this test had to send and receive a lot of CAN messages it caused
  // an error within the helpers

  // fifth test for port and mode edge cases
  prv_send_meta_data(1, 3, 7, 7, 0, 0, 0, TEST_BAUDRATE);

  prv_send_data(7);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(8, s_num_bytes);
  prv_test_equal_multiple_messages();

  TEST_ASSERT_EQUAL(SPI_PORT_TEST_2, s_spi_port);
  TEST_ASSERT_EQUAL(s_spi_settings_test_2.baudrate, s_spi_settings.baudrate);
  TEST_ASSERT_EQUAL(s_spi_settings_test_2.mode, s_spi_settings.mode);
  TEST_ASSERT_EQUAL(s_spi_settings_test_2.mosi.port, s_spi_settings.mosi.port);
  TEST_ASSERT_EQUAL(s_spi_settings_test_2.mosi.pin, s_spi_settings.mosi.pin);
  TEST_ASSERT_EQUAL(s_spi_settings_test_2.miso.port, s_spi_settings.miso.port);
  TEST_ASSERT_EQUAL(s_spi_settings_test_2.miso.pin, s_spi_settings.miso.pin);
  TEST_ASSERT_EQUAL(s_spi_settings_test_2.sclk.port, s_spi_settings.sclk.port);
  TEST_ASSERT_EQUAL(s_spi_settings_test_2.sclk.pin, s_spi_settings.sclk.pin);
  TEST_ASSERT_EQUAL(s_spi_settings_test_2.cs.port, s_spi_settings.cs.port);
  TEST_ASSERT_EQUAL(s_spi_settings_test_2.cs.pin, s_spi_settings.cs.pin);

  TEST_ASSERT_EQUAL(4, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_data_status[1]);

  prv_clear_received_data_array();
  s_num_bytes = 0;

  // sixth test for more than one return message
  prv_send_meta_data(0, 0, 8, 8, 0, 0, 0, TEST_BAUDRATE);

  prv_send_data(7);
  prv_send_data(1);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(10, s_num_bytes);
  prv_test_equal_multiple_messages();

  TEST_ASSERT_EQUAL(5, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_data_status[1]);

  prv_clear_received_data_array();
  s_num_bytes = 0;
}

void test_invalid_input(void) {
  // This test tests the different invalid inputs
  // for spi_exchange

  // first test invalid cs port
  prv_send_first_meta_data(0, 0, 7, 7, INVALID_CS_PORT, 0, 0);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_data_status[1]);

  // second test invalid cs pin
  prv_send_first_meta_data(0, 0, 7, 7, 0, INVALID_CS_PIN, 0);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_data_status[1]);

  // third test invalid port
  prv_send_first_meta_data(INVALID_SPI_PORT, 0, 7, 7, 0, 0, 0);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(3, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_data_status[1]);

  // fourth test invalid mode
  prv_send_first_meta_data(0, INVALID_SPI_MODE, 7, 7, 0, 0, 0);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(4, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_data_status[1]);
}

void test_timeout(void) {
  // This test tests the watchdog timeout error in spi_exchange

  // test for watchdog error with second meta data message
  prv_send_first_meta_data(0, 0, 7, 7, 0, 0, 0);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, s_received_data_status[1]);

  // test for watchdog error with data message
  prv_send_meta_data(0, 0, 7, 7, 0, 0, 0, TEST_BAUDRATE);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, s_received_data_status[1]);
}

StatusCode TEST_MOCK(spi_exchange)(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                                   size_t rx_len) {
  // This function mocks spi_exchange from spi.h

  for (size_t i = 0; i < rx_len; i++) rx_data[i] = 1;

  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(spi_init)(SpiPort spi, const SpiSettings *settings) {
  // This function mocks spi_init from spi.h

  s_spi_port = spi;
  s_spi_settings = *settings;

  return STATUS_CODE_OK;
}
