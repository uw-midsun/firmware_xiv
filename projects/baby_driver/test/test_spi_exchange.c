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

#define INVALID_SPI_PORT (2)
#define INVALID_SPI_MODE (4)

typedef enum {
  TEST_CAN_EVENT_TX = 0,
  TEST_CAN_EVENT_RX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

static CanStorage s_can_storage;

static uint8_t s_callback_counter_status;
static uint8_t s_num_messages;
static uint8_t s_received_data_status[8];
static uint8_t s_received_data[296];

static SpiSettings spi_settings;
static SpiPort spi_port;

static SpiSettings spi_settings_test_1 = { .baudrate = 0x005b8d80,
                                           .mode = SPI_MODE_0,
                                           .mosi = CONTROLLER_BOARD_ADDR_SPI1_MOSI,
                                           .miso = CONTROLLER_BOARD_ADDR_SPI1_MISO,
                                           .sclk = CONTROLLER_BOARD_ADDR_SPI1_SCK,
                                           .cs = (GpioAddress){ 0, 0 } };
#define SPI_PORT_TEST_1 (SPI_PORT_1)

static SpiSettings spi_settings_test_2 = { .baudrate = 0x005b8d80,
                                           .mode = SPI_MODE_3,
                                           .mosi = CONTROLLER_BOARD_ADDR_SPI2_MOSI,
                                           .miso = CONTROLLER_BOARD_ADDR_SPI2_MISO,
                                           .sclk = CONTROLLER_BOARD_ADDR_SPI2_SCK,
                                           .cs = CONTROLLER_BOARD_ADDR_SPI2_NSS };
#define SPI_PORT_TEST_2 (SPI_PORT_2)

static StatusCode prv_callback_spi_exchange_status(uint8_t data[8], void *context,
                                                   bool *tx_result) {
  memcpy(s_received_data_status, data, 8);
  *tx_result = false;
  s_callback_counter_status++;

  return STATUS_CODE_OK;
}

static StatusCode prv_callback_spi_exchange(uint8_t data[8], void *context, bool *tx_result) {
  for (int i = 0; i < 8; i++) {
    s_received_data[s_num_messages] = data[i];
    if (data[i] != 0) s_num_messages++;
  }
  *tx_result = false;

  return STATUS_CODE_OK;
}

static void send_meta_data(uint8_t port, uint8_t mode, uint8_t tx_len, uint8_t rx_len,
                           uint8_t cs_port, uint8_t cs_pin, uint8_t use_cs, uint32_t baudrate) {
  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_SPI_EXCHANGE_METADATA_1,  // id
                          port, mode,                                  // port, mode
                          tx_len, rx_len,                              // tx_len, rx_len
                          cs_port, cs_pin, use_cs);                    // cs_port, cs_pin, use_cs

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  uint8_t baudrate_1 = (baudrate & 0x000000ff);
  uint8_t baudrate_2 = (baudrate & 0x0000ff00) >> 8;
  uint8_t baudrate_3 = (baudrate & 0x00ff0000) >> 16;
  uint8_t baudrate_4 = (baudrate & 0xff000000) >> 24;

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_SPI_EXCHANGE_METADATA_2,      // id
                          baudrate_1, baudrate_2, baudrate_3, baudrate_4,  // baudrate
                          0, 0, 0);                                        // 3 * uint8 unused
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
}

static void send_first_meta_data(uint8_t port, uint8_t mode, uint8_t tx_len, uint8_t rx_len,
                                 uint8_t cs_port, uint8_t cs_pin, uint8_t use_cs) {
  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_SPI_EXCHANGE_METADATA_1,  // id
                          port, mode,                                  // port, mode
                          tx_len, rx_len,                              // tx_len, rx_len
                          cs_port, cs_pin, use_cs);                    // cs_port, cs_pin, use_cs
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
}

static void clear_received_data_array() {
  for (uint8_t i = 0; i < s_num_messages; i++) {
    s_received_data[i] = 0;
  }
}

static void test_equal_multiple_messages() {
  uint8_t mock_array[296];
  for (int i = 0; i < s_num_messages; i++) {
    if (i % 8 == 0) {
      mock_array[i] = BABYDRIVER_MESSAGE_SPI_EXCHANGE_RX_DATA;
    } else {
      mock_array[i] = 1;
    }
  }

  for (int i = 0; i < s_num_messages; i++) {
    TEST_ASSERT_EQUAL(mock_array[i], s_received_data[i]);
  }
}

static void send_data(uint8_t bytes) {
  if (bytes == 1)
    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_SPI_EXCHANGE_TX_DATA,  // id
                            1, 0, 0, 0, 0, 0, 0);
  else if (bytes == 3)
    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_SPI_EXCHANGE_TX_DATA,  // id
                            1, 1, 1, 0, 0, 0, 0);
  else if (bytes == 7)
    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_SPI_EXCHANGE_TX_DATA,  // id
                            1, 1, 1, 1, 1, 1, 1);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BABYDRIVER, TEST_CAN_EVENT_TX,
                                  TEST_CAN_EVENT_RX, TEST_CAN_EVENT_FAULT);
  TEST_ASSERT_OK(dispatcher_init());
  TEST_ASSERT_OK(spi_exchange_init());

  clear_received_data_array();
  memset(s_received_data_status, 0, sizeof(s_received_data_status));
  s_callback_counter_status = 0;
  s_num_messages = 0;

  TEST_ASSERT_OK(dispatcher_register_callback(BABYDRIVER_MESSAGE_STATUS,
                                              prv_callback_spi_exchange_status, NULL));
  TEST_ASSERT_OK(dispatcher_register_callback(BABYDRIVER_MESSAGE_SPI_EXCHANGE_RX_DATA,
                                              prv_callback_spi_exchange, NULL));
}
void teardown_test(void) {}

void test_valid_input(void) {
  // first test using the given cs port and pin
  send_meta_data(0, 0, 7, 7, 0, 0, 1, 0x005b8d80);

  send_data(7);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(8, s_num_messages);
  test_equal_multiple_messages();

  TEST_ASSERT_EQUAL(SPI_PORT_TEST_1, spi_port);
  TEST_ASSERT_EQUAL(spi_settings_test_1.baudrate, spi_settings.baudrate);
  TEST_ASSERT_EQUAL(spi_settings_test_1.mode, spi_settings.mode);
  TEST_ASSERT_EQUAL(spi_settings_test_1.mosi.port, spi_settings.mosi.port);
  TEST_ASSERT_EQUAL(spi_settings_test_1.mosi.pin, spi_settings.mosi.pin);
  TEST_ASSERT_EQUAL(spi_settings_test_1.miso.port, spi_settings.miso.port);
  TEST_ASSERT_EQUAL(spi_settings_test_1.miso.pin, spi_settings.miso.pin);
  TEST_ASSERT_EQUAL(spi_settings_test_1.sclk.port, spi_settings.sclk.port);
  TEST_ASSERT_EQUAL(spi_settings_test_1.sclk.pin, spi_settings.sclk.pin);
  TEST_ASSERT_EQUAL(spi_settings_test_1.cs.port, spi_settings.cs.port);
  TEST_ASSERT_EQUAL(spi_settings_test_1.cs.pin, spi_settings.cs.pin);

  TEST_ASSERT_EQUAL(1, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_data_status[1]);

  clear_received_data_array();
  s_num_messages = 0;

  // second test using the default cs port and pin
  send_meta_data(0, 0, 7, 7, 0, 0, 0, 0x005b8d80);

  send_data(7);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(8, s_num_messages);
  test_equal_multiple_messages();

  TEST_ASSERT_EQUAL(2, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_data_status[1]);

  clear_received_data_array();
  s_num_messages = 0;

  // third test for tx_len and rx_len edge cases (0)
  send_meta_data(0, 0, 0, 0, 0, 0, 0, 0x005b8d80);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(0, s_num_messages);

  TEST_ASSERT_EQUAL(3, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_data_status[1]);

  clear_received_data_array();
  s_num_messages = 0;

  /*
    // fourth test for tx_len and rx_len edge cases (255)
    send_meta_data(0, 0, 255, 255, 0, 0, 0, 0x005b8d80);

    for (uint8_t i = 0; i < 36; i++) {
      send_data(7);
    }
    send_data(3);
    for(uint8_t i = 0; i < 37; i++){
      MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
    }
    // right now there is no way to receive this many messages without the helper issue

    TEST_ASSERT_EQUAL(292, s_num_messages);
    test_equal_multiple_messages();

    clear_received_data_array();
    s_num_messages = 0;
  */

  // fifth test for port and mode edge cases
  send_meta_data(1, 3, 7, 7, 0, 0, 0, 0x005b8d80);

  send_data(7);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(8, s_num_messages);
  test_equal_multiple_messages();

  TEST_ASSERT_EQUAL(SPI_PORT_TEST_2, spi_port);
  TEST_ASSERT_EQUAL(spi_settings_test_2.baudrate, spi_settings.baudrate);
  TEST_ASSERT_EQUAL(spi_settings_test_2.mode, spi_settings.mode);
  TEST_ASSERT_EQUAL(spi_settings_test_2.mosi.port, spi_settings.mosi.port);
  TEST_ASSERT_EQUAL(spi_settings_test_2.mosi.pin, spi_settings.mosi.pin);
  TEST_ASSERT_EQUAL(spi_settings_test_2.miso.port, spi_settings.miso.port);
  TEST_ASSERT_EQUAL(spi_settings_test_2.miso.pin, spi_settings.miso.pin);
  TEST_ASSERT_EQUAL(spi_settings_test_2.sclk.port, spi_settings.sclk.port);
  TEST_ASSERT_EQUAL(spi_settings_test_2.sclk.pin, spi_settings.sclk.pin);
  TEST_ASSERT_EQUAL(spi_settings_test_2.cs.port, spi_settings.cs.port);
  TEST_ASSERT_EQUAL(spi_settings_test_2.cs.pin, spi_settings.cs.pin);

  TEST_ASSERT_EQUAL(4, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_data_status[1]);

  clear_received_data_array();
  s_num_messages = 0;

  // sixth test for more than one return message
  send_meta_data(0, 0, 8, 8, 0, 0, 0, 0x005b8d80);

  send_data(7);
  send_data(1);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(10, s_num_messages);
  test_equal_multiple_messages();

  TEST_ASSERT_EQUAL(5, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_data_status[1]);

  clear_received_data_array();
  s_num_messages = 0;
}

void test_invalid_input(void) {
  // first test invalid cs port
  send_first_meta_data(0, 0, 7, 7, INVALID_CS_PORT, 0, 0);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_data_status[1]);

  // second test invalid cs pin
  send_first_meta_data(0, 0, 7, 7, 0, INVALID_CS_PIN, 0);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_data_status[1]);

  // third test invalid port
  send_first_meta_data(INVALID_SPI_PORT, 0, 7, 7, 0, 0, 0);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(3, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_data_status[1]);

  // fourth test invalid mode
  send_first_meta_data(0, INVALID_SPI_MODE, 7, 7, 0, 0, 0);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(4, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_data_status[1]);
}

void test_timeout(void) {
  // test for watchdog error with second meta data message
  send_first_meta_data(0, 0, 7, 7, 0, 0, 0);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, s_received_data_status[1]);

  // test for watchdog error with data message
  send_meta_data(0, 0, 7, 7, 0, 0, 0, 0x005b8d80);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_callback_counter_status);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data_status[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, s_received_data_status[1]);
}

StatusCode TEST_MOCK(spi_exchange)(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                                   size_t rx_len) {
  for (uint8_t i = 0; i < rx_len; i++) rx_data[i] = 1;

  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(spi_init)(SpiPort spi, const SpiSettings *settings) {
  // still need to add tests making sure the settings and port are the same
  spi_port = spi;
  spi_settings = *settings;

  return STATUS_CODE_OK;
}
