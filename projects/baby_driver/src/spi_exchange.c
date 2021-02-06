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
#include "spi_mcu.h"
#include "status.h"
#include "watchdog.h"

static SpiPort s_port;
static SpiMode s_mode;
static uint8_t s_tx_len = 0;
static uint8_t s_rx_len = 0;
static uint8_t s_cs_port;
static uint8_t s_cs_pin;
static bool s_use_cs;

static uint32_t s_baudrate;

static size_t s_bytes_received = 0;
static size_t s_bytes_sent = 0;

/*
    The two arrays are declared to be 260 to prevent an overflow
    My module can read up to s_rx_data[259] but 256 to 259 will all be 0
*/
static uint8_t s_tx_data[260] = { 0 };
static uint8_t s_rx_data[260] = { 0 };

static WatchdogStorage s_watchdog_storage = { 0 };
static WatchdogTimeout s_timeout_ms;
static bool s_in_transaction;

static uint32_t s_soft_timer_delay;

static StatusCode prv_spi_set_up(void) {
  /*
      This function sets up the spi_settings for spi_init
      according to the data received from the python messages
  */

  GpioAddress mosi = CONTROLLER_BOARD_ADDR_SPI1_MOSI;
  GpioAddress miso = CONTROLLER_BOARD_ADDR_SPI1_MISO;
  GpioAddress sclk = CONTROLLER_BOARD_ADDR_SPI1_SCK;
  GpioAddress cs = CONTROLLER_BOARD_ADDR_SPI1_NSS;

  if (s_port == 1) {
    mosi = (GpioAddress)CONTROLLER_BOARD_ADDR_SPI2_MOSI;
    miso = (GpioAddress)CONTROLLER_BOARD_ADDR_SPI2_MISO;
    sclk = (GpioAddress)CONTROLLER_BOARD_ADDR_SPI2_SCK;
    cs = (GpioAddress)CONTROLLER_BOARD_ADDR_SPI2_NSS;
  }

  if (s_use_cs) {
    cs = (GpioAddress){ s_cs_port, s_cs_pin };
  }

  SpiSettings spi_settings = {
    .baudrate = s_baudrate, .mode = s_mode, .mosi = mosi, .miso = miso, .sclk = sclk, .cs = cs
  };
  status_ok_or_return(spi_init(s_port, &spi_settings));

  return STATUS_CODE_OK;
}

static void prv_clear_array(uint8_t *array, size_t length) {
  /*
      This function clears the array it is given
  */
  for (size_t i = 0; i < length; i++) {
    array[i] = 0;
  }
}

static void prv_reset_spi_exchange(void) {
  /*
      This function resets everything in spi_exchange
      so that it is ready for the next spi_exchange python call
  */
  prv_clear_array(s_tx_data, s_tx_len);
  prv_clear_array(s_rx_data, s_rx_len);
  s_bytes_received = 0;
  s_bytes_sent = 0;
  s_tx_len = 0;
  s_rx_len = 0;
}

static void prv_timer_status_callback(SoftTimerId timer_id, void *context) {
  /*
      This function is used to send a STATUS_CODE_OK message
      The function is in a soft timer call back because there
      needs to be time between each babydriver message sent
  */
  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_STATUS, STATUS_CODE_OK, 0, 0, 0, 0, 0, 0);
}

static void prv_timer_data_callback(SoftTimerId timer_id, void *context) {
  /*
      This function is used to send the data messages back to the python
      The function is in a soft timer call back because there
      needs to be time between each babydriver message sent
  */
  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_SPI_EXCHANGE_RX_DATA, s_rx_data[s_bytes_sent],
                          s_rx_data[s_bytes_sent + 1], s_rx_data[s_bytes_sent + 2],
                          s_rx_data[s_bytes_sent + 3], s_rx_data[s_bytes_sent + 4],
                          s_rx_data[s_bytes_sent + 5], s_rx_data[s_bytes_sent + 6]);
  s_bytes_sent += 7;
  if (s_bytes_sent < s_rx_len) {
    soft_timer_start(s_soft_timer_delay, prv_timer_data_callback, NULL, NULL);
  } else {
    s_in_transaction = false;
    soft_timer_start(s_soft_timer_delay, prv_timer_status_callback, NULL, NULL);
    prv_reset_spi_exchange();
  }
}

static StatusCode prv_send_can_rx_msgs(void) {
  /*
      This function calls the set up (for spi_init)
      and calls the spi_exchange function
  */
  status_ok_or_return(prv_spi_set_up());
  status_ok_or_return(spi_exchange(s_port, s_tx_data, s_tx_len, s_rx_data, s_rx_len));
  if (s_rx_len != 0) {
    soft_timer_start(s_soft_timer_delay, prv_timer_data_callback, NULL, NULL);
  } else {
    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_STATUS, STATUS_CODE_OK, 0, 0, 0, 0, 0, 0);
  }
  return STATUS_CODE_OK;
}

static StatusCode prv_callback_spi_exchange_metadata1(uint8_t data[8], void *context,
                                                      bool *tx_result) {
  /*
      This function is the dispatcher callback when the module
      receives a babydriver can message with the id
      BABYDRIVER_MESSAGE_SPI_EXCHANGE_METADATA_1
  */
  watchdog_kick(&s_watchdog_storage);
  s_in_transaction = true;
  *tx_result = false;

  s_port = data[1];
  s_mode = data[2];
  s_tx_len = data[3];
  s_rx_len = data[4];
  s_cs_port = data[5];
  s_cs_pin = data[6];
  s_use_cs = data[7] != 0;

  if (s_port >= NUM_SPI_PORTS) {
    *tx_result = true;
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_cs_port >= NUM_GPIO_PORTS || s_cs_pin >= GPIO_PINS_PER_PORT) {
    *tx_result = true;
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_mode >= NUM_SPI_MODES) {
    *tx_result = true;
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

static StatusCode prv_callback_spi_exchange_metadata2(uint8_t data[8], void *context,
                                                      bool *tx_result) {
  /*
      This function is the dispatcher callback when the module
      receives a babydriver can message with the id
      BABYDRIVER_MESSAGE_SPI_EXCHANGE_METADATA_2
  */
  watchdog_kick(&s_watchdog_storage);
  s_in_transaction = true;
  *tx_result = false;

  // Decoding baudrate from little endian format
  s_baudrate = 0;
  for (size_t i = 0; i < 4; i++) {
    s_baudrate |= (uint32_t)data[i + 1] << (8 * (i));
  }

  if (s_tx_len == 0) {
    *tx_result = true;
    status_ok_or_return(prv_send_can_rx_msgs());
  }
  *tx_result = false;

  return STATUS_CODE_OK;
}

static StatusCode prv_callback_spi_exchange_python_transfer(uint8_t data[8], void *context,
                                                            bool *tx_result) {
  /*
      This function is the dispatcher callback when the module
      receives a babydriver can message with the id
      BABYDRIVER_MESSAGE_SPI_EXCHANGE_TX_DATA
  */
  watchdog_kick(&s_watchdog_storage);
  s_in_transaction = true;
  *tx_result = false;

  for (uint8_t i = 1; i < 8; i++) {
    if (s_bytes_received == s_tx_len) break;
    s_tx_data[s_bytes_received] = data[i];
    s_bytes_received++;
  }

  if (s_bytes_received == s_tx_len) {
    *tx_result = true;
    status_ok_or_return(prv_send_can_rx_msgs());
  }
  *tx_result = false;

  return STATUS_CODE_OK;
}

static void prv_timeout_callback(void *context) {
  /*
      This function is the watchdog callback
      It sends a STATUS_CODE_TIMEOUT if the module
      does not receive all of the messages it was
      supposed to receive
  */
  if (s_in_transaction) {
    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_STATUS, STATUS_CODE_TIMEOUT, 0, 0, 0, 0, 0, 0);
  }
  prv_reset_spi_exchange();
}

StatusCode spi_exchange_init(uint32_t spi_exchange_timeout, uint32_t tx_delay) {
  /*
      This function starts the spi_exchange module
      The two parameters are the timeout length for watchdog
      and the delay between each CAN message sent back to the python
  */
  s_timeout_ms = spi_exchange_timeout;
  s_soft_timer_delay = tx_delay;
  watchdog_start(&s_watchdog_storage, s_timeout_ms, prv_timeout_callback, NULL);

  status_ok_or_return(dispatcher_register_callback(BABYDRIVER_MESSAGE_SPI_EXCHANGE_METADATA_1,
                                                   prv_callback_spi_exchange_metadata1, NULL));

  status_ok_or_return(dispatcher_register_callback(BABYDRIVER_MESSAGE_SPI_EXCHANGE_METADATA_2,
                                                   prv_callback_spi_exchange_metadata2, NULL));

  status_ok_or_return(dispatcher_register_callback(
      BABYDRIVER_MESSAGE_SPI_EXCHANGE_TX_DATA, prv_callback_spi_exchange_python_transfer, NULL));

  return STATUS_CODE_OK;
}
