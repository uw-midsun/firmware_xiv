#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "bootloader_events.h"
#include "can_datagram.h"
#include "can_unpack_impl.h"
#include "crc32.h"
#include "delay.h"
#include "dispatcher.h"
#include "event_queue.h"
#include "fifo.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "ping.h"
#include "soft_timer.h"
#include "status.h"
#include "string.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CLIENT_SCRIPT_ID 0
#define TEST_TX_FIFO_SIZE 32

static uint8_t s_client_id = 0;
static uint8_t s_board_id = 2;

static CanStorage s_test_can_storage;
static CanSettings s_test_can_settings = {
  .loopback = true,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = CAN_DATAGRAM_EVENT_RX,
  .tx_event = CAN_DATAGRAM_EVENT_TX,
  .fault_event = CAN_DATAGRAM_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
};

static CanDatagramSettings s_test_datagram_settings = {
  .tx_event = DATAGRAM_EVENT_TX,
  .rx_event = DATAGRAM_EVENT_RX,
  .repeat_event = DATAGRAM_EVENT_REPEAT,
  .error_event = DATAGRAM_EVENT_ERROR,
  .error_cb = NULL,
};

#define TEST_DATA_LEN 32

typedef struct TestCanDatagramMessage {
  uint8_t data[8];  // 8 is the max len per CAN msg
  size_t len;
  bool is_start_msg;
} TestCanDatagramMessage;

static uint8_t s_tx_data[TEST_DATA_LEN] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                            'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p' };

static uint8_t s_rx_data[DGRAM_MAX_DATA_SIZE];
static uint16_t s_rx_data_len;

static CanMessage s_tx_buffer[TEST_TX_FIFO_SIZE];
static Fifo s_tx_fifo;
static bool s_tx_cmpl;
static CanDatagramExitCb s_tx_cmpl_cb;

static StatusCode prv_tx_to_fifo(uint8_t *data, size_t len, bool is_start_message) {
  if (len > 8) {
    return STATUS_CODE_INVALID_ARGS;
  }
  CanMessage message = {
    .source_id = SYSTEM_CAN_DEVICE_BOOTLOADER,
    .msg_id = TEST_CLIENT_SCRIPT_ID,
    .type = is_start_message ? CAN_MSG_TYPE_ACK : CAN_MSG_TYPE_DATA,
    .dlc = len,
  };
  // copy the message data over to the message to be transmitted
  memcpy(message.data_u8, data, len);
  fifo_push(&s_tx_fifo, &message);
  return STATUS_CODE_OK;
}

static void prv_tx_from_fifo(void) {
  CanMessage message;
  fifo_pop(&s_tx_fifo, &message);
  can_transmit(&message, NULL);
}

static void prv_mock_tx_compl(void) {
  s_tx_cmpl = true;
}

static void prv_send_tx_to_fifo(CanDatagramTxConfig tx_config) {
  fifo_init(&s_tx_fifo, s_tx_buffer);
  s_tx_cmpl = false;

  s_tx_cmpl_cb = tx_config.tx_cmpl_cb;
  tx_config.tx_cmpl_cb = prv_mock_tx_compl;
  tx_config.tx_cb = prv_tx_to_fifo;
  can_datagram_start_tx(&tx_config);

  Event e = { 0 };
  while (!s_tx_cmpl) {
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
    can_process_event(&e);
  }  // puts all messages that are "sent" into the fifo
  s_tx_cmpl_cb();
}

static TestCanDatagramMessage s_rx_buffer[TEST_TX_FIFO_SIZE];
static Fifo s_rx_fifo;

static StatusCode prv_rx_to_fifo(uint8_t *data, size_t len, bool is_start_msg) {
  if (is_start_msg) {
    fifo_init(&s_rx_fifo, s_rx_buffer);
  }
  TestCanDatagramMessage msg = {
    .len = len,
    .is_start_msg = is_start_msg,
  };
  memcpy(msg.data, data, len);
  return fifo_push(&s_rx_fifo, &msg);
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  crc32_init();

  bootloader_can_init(&s_test_can_storage, &s_test_can_settings, s_board_id);
  can_datagram_init(&s_test_datagram_settings);
  dispatcher_init(s_board_id);
}

void teardown_test(void) {}

void test_ping(void) {
  TEST_ASSERT_OK(ping_init(s_board_id));

  CanDatagramTxConfig tx_config = {
    .dgram_type = BOOTLOADER_DATAGRAM_PING_COMMAND,
    .destination_nodes_len = 1,
    .destination_nodes = &s_board_id,
    .data_len = 0,
    .data = NULL,
    .tx_cmpl_cb = tx_cmpl_cb,
  };
  prv_send_tx_to_fifo(tx_config);

  TEST_ASSERT_OK(bootloader_can_register_debug_handler(prv_rx_to_fifo));

  Event e = { 0 };
  prv_tx_from_fifo();
  MS_TEST_HELPER_CAN_TX_RX(CAN_DATAGRAM_EVENT_TX, CAN_DATAGRAM_EVENT_RX);

  size_t tx_left = fifo_size(&s_tx_fifo);
  size_t response_size = 0;
  bool send_tx = true;
  while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {
    // this loop will loop through both command rx and response tx
    if (send_tx && tx_left > 0) {
      prv_tx_from_fifo();
      send_tx = false;
      tx_left--;
    }
    MS_TEST_HELPER_AWAIT_EVENT(e);
    if (e.id == CAN_DATAGRAM_EVENT_TX) {
      send_tx = true;
    }
    can_process_event(&e);
    can_datagram_process_event(&e);
  }
  // LOG_DEBUG("rx size: %li\n", fifo_size(&s_rx_fifo));  // should be 3 if rx worked
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_TX_COMPLETE, can_datagram_get_status());

  uint8_t s_destination_nodes[DGRAM_MAX_DEST_NODES_SIZE] = { 0 };
  CanDatagramRxConfig rx_config = {
    .destination_nodes = s_destination_nodes,
    .data = s_rx_data,
    .node_id = 0,  // listen to all
    .rx_cmpl_cb = NULL,
  };
  can_datagram_start_listener(&rx_config);
  TestCanDatagramMessage msg;
  fifo_pop(&s_rx_fifo, &msg);
  can_datagram_rx(msg.data, msg.len, msg.is_start_msg);
  MS_TEST_HELPER_AWAIT_EVENT(e);
  can_datagram_process_event(&e);

  while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {
    // listens from s_rx-fifo
    TestCanDatagramMessage msg;
    fifo_pop(&s_rx_fifo, &msg);
    can_datagram_rx(msg.data, msg.len, msg.is_start_msg);
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
  TEST_ASSERT_EQUAL(s_board_id, rx_config.data[0]);
  TEST_ASSERT_EQUAL(BOOTLOADER_DATAGRAM_PING_RESPONSE, rx_config.dgram_type);
}

void test_ping_addressed_to_multiple(void) {
  TEST_ASSERT_OK(ping_init(s_board_id));

  uint8_t multiple_addr[5] = { 1, s_board_id, 3, 4, 5 };
  CanDatagramTxConfig tx_config = {
    .dgram_type = BOOTLOADER_DATAGRAM_PING_COMMAND,
    .destination_nodes_len = 5,
    .destination_nodes = multiple_addr,
    .data_len = 0,
    .data = NULL,
    .tx_cmpl_cb = tx_cmpl_cb,
  };
  prv_send_tx_to_fifo(tx_config);

  TEST_ASSERT_OK(bootloader_can_register_debug_handler(prv_rx_to_fifo));

  Event e = { 0 };
  prv_tx_from_fifo();
  MS_TEST_HELPER_CAN_TX_RX(CAN_DATAGRAM_EVENT_TX, CAN_DATAGRAM_EVENT_RX);

  size_t tx_left = fifo_size(&s_tx_fifo);
  size_t response_size = 0;
  bool send_tx = true;
  while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {
    // this loop will loop through both command rx and response tx
    if (send_tx && tx_left > 0) {
      prv_tx_from_fifo();
      send_tx = false;
      tx_left--;
    }
    MS_TEST_HELPER_AWAIT_EVENT(e);
    if (e.id == CAN_DATAGRAM_EVENT_TX) {
      send_tx = true;
    }
    can_process_event(&e);
    can_datagram_process_event(&e);
  }
  // LOG_DEBUG("rx size: %li\n", fifo_size(&s_rx_fifo));  // should be 3 if rx worked...
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_TX_COMPLETE, can_datagram_get_status());

  uint8_t s_destination_nodes[DGRAM_MAX_DEST_NODES_SIZE] = { 0 };
  CanDatagramRxConfig rx_config = {
    .destination_nodes = s_destination_nodes,
    .data = s_rx_data,
    .node_id = 0,  // listen to all
    .rx_cmpl_cb = NULL,
  };
  can_datagram_start_listener(&rx_config);
  TestCanDatagramMessage msg;
  fifo_pop(&s_rx_fifo, &msg);
  can_datagram_rx(msg.data, msg.len, msg.is_start_msg);
  MS_TEST_HELPER_AWAIT_EVENT(e);
  can_datagram_process_event(&e);

  while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {
    // listens from s_rx-fifo
    TestCanDatagramMessage msg;
    fifo_pop(&s_rx_fifo, &msg);
    can_datagram_rx(msg.data, msg.len, msg.is_start_msg);
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
  TEST_ASSERT_EQUAL(s_board_id, rx_config.data[0]);
  TEST_ASSERT_EQUAL(BOOTLOADER_DATAGRAM_PING_RESPONSE, rx_config.dgram_type);
}
