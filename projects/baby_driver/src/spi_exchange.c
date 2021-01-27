#include "spi_exchange.h"

#include "log.h"

#include "babydriver_msg_defs.h"
#include "can.h"
#include "can_transmit.h"
#include "controller_board_pins.h"
#include "dispatcher.h"
#include "gpio_mcu.h"
#include "math.h"
#include "spi.h"
#include "status.h"
#include "watchdog.h"

static uint8_t port;
static uint8_t mode;
static uint8_t tx_len;
static uint8_t rx_len;
static uint8_t cs_port;
static uint8_t cs_pin;
static uint8_t use_cs;

static uint32_t baudrate;

static uint8_t number_of_data_msgs = 0;
static uint8_t tx_array_loc = 0;
static uint8_t tx_data[255] = { 0 };
static uint8_t *p_tx_data = tx_data;

static uint8_t rx_data[255] = { 0 };
static uint8_t *p_rx_data = rx_data;

static WatchdogStorage watchdog_storage = { 0 };
static WatchdogTimeout timeout_ms = 2000;
static bool stop_watchdog;

static StatusCode prv_callback_spi_exchange_metadata1(uint8_t data[8], void *context,
                                                      bool *tx_result) {
  // LOG_DEBUG("WORKS1 \n");

  watchdog_kick(&watchdog_storage);
  stop_watchdog = false;
  *tx_result = false;

  port = data[1];
  mode = data[2];
  tx_len = data[3];
  rx_len = data[4];
  cs_port = data[5];
  cs_pin = data[6];
  use_cs = data[7];

  if (port > 1) {
    *tx_result = true;
    return STATUS_CODE_INVALID_ARGS;
  }

  if (cs_port >= NUM_GPIO_PORTS || cs_pin >= GPIO_PINS_PER_PORT) {
    *tx_result = true;
    return STATUS_CODE_INVALID_ARGS;
  }

  if (mode > 3) {
    *tx_result = true;
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

static StatusCode prv_callback_spi_exchange_metadata2(uint8_t data[8], void *context,
                                                      bool *tx_result) {
  // LOG_DEBUG("WORKS2 \n");

  watchdog_kick(&watchdog_storage);
  stop_watchdog = false;
  *tx_result = false;

  baudrate = 0;
  uint32_t temp = 0;
  for (int i = 4; i > 0; i--) {
    temp = (uint32_t)data[i];
    temp = temp << (8 * (i - 1));
    baudrate = baudrate | temp;
    temp = 0;
  }
  return STATUS_CODE_OK;
}

static void prv_spi_set_up(void) {
  GpioAddress mosi = CONTROLLER_BOARD_ADDR_SPI1_MOSI;
  GpioAddress miso = CONTROLLER_BOARD_ADDR_SPI1_MISO;
  GpioAddress sclk = CONTROLLER_BOARD_ADDR_SPI1_SCK;
  GpioAddress cs = CONTROLLER_BOARD_ADDR_SPI1_NSS;

  if (port == 1) {
    mosi = (GpioAddress)CONTROLLER_BOARD_ADDR_SPI2_MOSI;
    miso = (GpioAddress)CONTROLLER_BOARD_ADDR_SPI2_MISO;
    sclk = (GpioAddress)CONTROLLER_BOARD_ADDR_SPI2_SCK;
    cs = (GpioAddress)CONTROLLER_BOARD_ADDR_SPI2_NSS;
  }

  if (use_cs != 0) {
    cs = (GpioAddress){ cs_port, cs_pin };
  }

  SpiSettings spi_settings = {
    .baudrate = baudrate, .mode = mode, .mosi = mosi, .miso = miso, .sclk = sclk, .cs = cs
  };
  spi_init(port, &spi_settings);
}

static void clear_array(uint8_t *array, uint8_t length) {
  for (int i = 0; i < length; i++) {
    array[i] = 0;
  }
}

static void reset_spi_exchange() {
  clear_array(p_tx_data, tx_len);
  clear_array(p_rx_data, rx_len);
  number_of_data_msgs = 0;
  tx_array_loc = 0;
}

static StatusCode prv_callback_spi_exchange_python_transfer(uint8_t data[8], void *context,
                                                            bool *tx_result) {
  // LOG_DEBUG("WORKS3 \n");

  watchdog_kick(&watchdog_storage);
  stop_watchdog = false;
  *tx_result = false;

  number_of_data_msgs++;
  for (uint8_t i = 1; i < 8; i++) {
    tx_data[tx_array_loc] = data[i];
    tx_array_loc++;
  }

  if (number_of_data_msgs == ceil(tx_len / 7)) {
    *tx_result = true;
    prv_spi_set_up();
    spi_exchange(port, p_tx_data, tx_len, p_rx_data, rx_len);
    for (uint8_t i = 0; i < ceil(rx_len / 7); i++) {
      status_ok_or_return(
          CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_SPI_EXCHANGE_RX_DATA, rx_data[7 * i],
                                  rx_data[7 * i + 1], rx_data[7 * i + 2], rx_data[7 * i + 3],
                                  rx_data[7 * i + 4], rx_data[7 * i + 5], rx_data[7 * i + 6]));
    }
    stop_watchdog = true;
    reset_spi_exchange();
  }
  *tx_result = false;

  return STATUS_CODE_OK;
}

static void prv_timeout_callback(void *context) {
  if (!stop_watchdog)
    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_STATUS, STATUS_CODE_TIMEOUT, 0, 0, 0, 0, 0, 0);
  reset_spi_exchange();
}

StatusCode spi_exchange_init(void) {
  watchdog_start(&watchdog_storage, timeout_ms, prv_timeout_callback, NULL);

  status_ok_or_return(dispatcher_register_callback(BABYDRIVER_MESSAGE_SPI_EXCHANGE_METADATA_1,
                                                   prv_callback_spi_exchange_metadata1, NULL));

  status_ok_or_return(dispatcher_register_callback(BABYDRIVER_MESSAGE_SPI_EXCHANGE_METADATA_2,
                                                   prv_callback_spi_exchange_metadata2, NULL));

  status_ok_or_return(dispatcher_register_callback(
      BABYDRIVER_MESSAGE_SPI_EXCHANGE_TX_DATA, prv_callback_spi_exchange_python_transfer, NULL));

  return STATUS_CODE_OK;
}
