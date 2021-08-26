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

static EventId s_can_tx_event;

typedef struct DatagramMsg {
  union {
    uint64_t data;
    uint32_t data_u32[2];
    uint16_t data_u16[4];
    uint8_t data_u8[8];
  };
  size_t len;
  bool is_start_msg;
} DatagramMsg;

// rx and tx uses the same fifo, buffer, and cmpl_cb storage
// the fifo is not persistent across individual rx and tx helper calls
// they are global as required by prv_dgram_to_fifo and prv_dgram_to_fifo_cmpl_cb
static bool s_cmpl;
static CanDatagramExitCb s_cmpl_cb;
static Fifo s_dgram_can_fifo;
static DatagramMsg s_dgram_can_buffer[260];  // 260 is enough to fix a max sized datagram

// send all the datagram can msgs to fifo
static StatusCode prv_dgram_to_fifo(uint8_t *data, size_t len, bool is_start_message) {
  DatagramMsg message = { 0 };
  // a start message reinitalize the fifo and changes the msg type
  if (is_start_message) {
    // empty fifo
    while (fifo_size(&s_dgram_can_fifo) > 0) {
      fifo_pop(&s_dgram_can_fifo, &message);
    }
  }
  message.len = len;
  message.is_start_msg = is_start_message;
  memcpy(message.data_u8, data, len);

  fifo_push(&s_dgram_can_fifo, &message);
  return STATUS_CODE_OK;
}

static void prv_dgram_to_fifo_cmpl_cb(void) {
  s_cmpl = true;
  if (s_cmpl_cb != NULL) {
    s_cmpl_cb();
  }
}

static StatusCode prv_send_dgram_msg(DatagramMsg *dgram_msg) {
  CanMessage message = {
    .source_id = SYSTEM_CAN_DEVICE_BOOTLOADER,
    .msg_id = TEST_CLIENT_SCRIPT_ID,
    .data = dgram_msg->data,
    .dlc = dgram_msg->len,
    .type = dgram_msg->is_start_msg ? CAN_MSG_TYPE_ACK : CAN_MSG_TYPE_DATA,
  };
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
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_TX_COMPLETE, can_datagram_get_status());

  // send can messages from tx_fifo
  DatagramMsg message;
  while (fifo_size(&s_dgram_can_fifo) > 0) {
    fifo_pop(&s_dgram_can_fifo, &message);
    prv_send_dgram_msg(&message);
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

  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_TX_COMPLETE, can_datagram_get_status());

  // sends all data from fifo to can_datagram
  can_datagram_start_listener(rx_config);
  DatagramMsg message;
  while (fifo_size(&s_dgram_can_fifo) > 0) {
    fifo_pop(&s_dgram_can_fifo, &message);
    can_datagram_rx(message.data_u8, message.len, message.is_start_msg);
  }
  while (!s_cmpl) {
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());

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
  fifo_init(&s_dgram_can_fifo, s_dgram_can_buffer);
}
