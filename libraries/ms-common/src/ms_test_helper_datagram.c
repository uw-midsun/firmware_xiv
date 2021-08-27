#include "ms_test_helper_datagram.h"

#include "crc32.h"
#include "event_queue.h"
#include "fifo.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "string.h"

#define TEST_CLIENT_SCRIPT_ID 0
#define CAN_BUFFER_SIZE 8

static EventId s_can_tx_event;

// rx and tx uses the same fifo, buffer, and cmpl_cb storage
// the fifo is not persistent across individual rx and tx helper calls
// they are global as required by prv_dgram_to_fifo and prv_dgram_to_fifo_cmpl_cb
static bool s_cmpl;
static CanDatagramExitCb s_cmpl_cb;
static Fifo s_dgram_data_fifo;
static uint8_t s_dgram_data_buffer[2060];  // 2058 is enough to fix a max sized datagram
static bool s_transmit_start_msg;

// sends a datagram msg into fifo
static StatusCode prv_dgram_to_fifo(uint8_t *data, size_t len, bool is_start_message) {
  // a start message reinitalize the fifo and changes the msg type
  if (is_start_message) {  // empty fifo
    return fifo_init(&s_dgram_data_fifo, s_dgram_data_buffer);
  }
  return fifo_push_arr(&s_dgram_data_fifo, data, len);
}

static StatusCode prv_fifo_to_dgram(BootloaderCanCallback transmiter) {
  if (s_transmit_start_msg) {
    s_transmit_start_msg = false;
    return transmiter(0, 0, true);
  }
  uint8_t data[CAN_BUFFER_SIZE];
  size_t len = MIN(fifo_size(&s_dgram_data_fifo), (size_t)CAN_BUFFER_SIZE);
  fifo_pop_arr(&s_dgram_data_fifo, data, len);
  return transmiter(data, len, false);
}

// callback when datagram is finished with tx or rx-ing a datagram
static void prv_dgram_to_fifo_cmpl_cb(void) {
  s_cmpl = true;
  if (s_cmpl_cb != NULL) s_cmpl_cb();
}

// sends a can message as a client script, effectively the same as bootloader_can_transmit
// but with a different msg_id
static StatusCode prv_tx_as_client(uint8_t *data, size_t len, bool is_start_msg) {
  CanMessage message = {
    .source_id = SYSTEM_CAN_DEVICE_BOOTLOADER,
    .msg_id = TEST_CLIENT_SCRIPT_ID,
    .dlc = len,
    .type = is_start_msg ? CAN_MSG_TYPE_ACK : CAN_MSG_TYPE_DATA,
  };
  memcpy(message.data_u8, data, len);
  return can_transmit(&message, NULL);
}

// mock transmit a datagram from another device
StatusCode mock_tx_datagram(CanDatagramTxConfig *tx_config) {
  tx_config->tx_cb = prv_dgram_to_fifo;
  s_cmpl = false;

  s_cmpl_cb = tx_config->tx_cmpl_cb;
  tx_config->tx_cmpl_cb = prv_dgram_to_fifo_cmpl_cb;
  can_datagram_start_tx(tx_config);

  // send the datagram to tx_fifo
  Event e = { 0 };
  while (!s_cmpl) {
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
  }

  // send messages from fifo to can
  s_transmit_start_msg = true;
  while (fifo_size(&s_dgram_data_fifo) > 0) {
    prv_fifo_to_dgram(prv_tx_as_client);
    do {  // until another tx event is processed
      MS_TEST_HELPER_AWAIT_EVENT(e);
      can_datagram_process_event(&e);
      can_process_event(&e);
    } while (e.id != s_can_tx_event);
  }
  return STATUS_CODE_OK;
}

// mock recieve a datagram as another device
StatusCode mock_rx_datagram(CanDatagramRxConfig *rx_config) {
  s_cmpl_cb = rx_config->rx_cmpl_cb;
  s_cmpl = false;
  rx_config->rx_cmpl_cb = prv_dgram_to_fifo_cmpl_cb;

  // process the response to fifo
  Event e = { 0 };
  bootloader_can_register_debug_handler(prv_dgram_to_fifo);
  do {
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
    can_process_event(&e);
  } while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE);

  can_datagram_start_listener(rx_config);
  // send data from fifo to can_datagram
  s_transmit_start_msg = true;
  while (!s_cmpl) {
    prv_fifo_to_dgram(can_datagram_rx);
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
  }
  return STATUS_CODE_OK;
}

// initialize the datagram helper and dependencies
void init_datagram_helper(CanStorage *can_storage, CanSettings *can_settings, uint8_t board_id,
                          CanDatagramSettings *can_datagram_settings) {
  event_queue_init();
  interrupt_init();
  gpio_init();
  soft_timer_init();
  crc32_init();

  bootloader_can_init(can_storage, can_settings, board_id);
  can_datagram_init(can_datagram_settings);

  // used to send tx from fifo only when the last tx is processed
  // prevents overflowing the event queue
  s_can_tx_event = can_storage->tx_event;
  fifo_init(&s_dgram_data_fifo, s_dgram_data_buffer);
}
