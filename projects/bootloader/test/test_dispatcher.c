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
#include "soft_timer.h"
#include "status.h"
#include "string.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_DATA_GRAM_ID 2
#define TEST_CLIENT_SCRIPT_ID 0
#define TEST_TX_FIFO_SIZE 32

static uint8_t s_client_id = 0;
static uint8_t s_board_id = 1;

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
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_TX_COMPLETE, can_datagram_get_status());
  s_tx_cmpl_cb();
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  crc32_init();

  bootloader_can_init(&s_test_can_storage, &s_test_can_settings, s_board_id);
  can_datagram_init(&s_test_datagram_settings);
}

void teardown_test(void) {}

StatusCode prv_dispatch_check_cb(uint8_t *data, uint16_t len, void *context) {
  LOG_DEBUG("Callback called!\n");
  bool *cb_called = context;
  *cb_called = true;

  memcpy(s_rx_data, data, len);
  s_rx_data_len = len;
  return STATUS_CODE_OK;
}

void test_dispatch_calls_cb(void) {
  // tests basic dispatch calls the callback when datagram is recieved
  // and tests dispatch's tx_cmpl_cb resumes datagram rx correctly
  TEST_ASSERT_OK(dispatcher_init(s_board_id));

  CanDatagramTxConfig tx_config = {
    .dgram_type = TEST_DATA_GRAM_ID,
    .destination_nodes_len = 1,
    .destination_nodes = &s_board_id,
    .data_len = 0,
    .data = NULL,
    .tx_cmpl_cb = tx_cmpl_cb,
  };
  prv_send_tx_to_fifo(tx_config);
  // datagram is now in s_tx_fifo

  // bool cb_called = false;
  // TEST_ASSERT_OK(
  //     dispatcher_register_callback(TEST_DATA_GRAM_ID, prv_dispatch_check_cb, &cb_called));

  // Event e = { 0 };
  // prv_tx_from_fifo();  // start msg (first msg in fifo)
  // MS_TEST_HELPER_CAN_TX_RX(CAN_DATAGRAM_EVENT_TX, CAN_DATAGRAM_EVENT_RX);

  // // datagram will be in rx mode while we send the datagram with can from fifo.
  // bool send_tx = true;
  // while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {  // Loop until rx complete
  //   if (send_tx) {
  //     prv_tx_from_fifo();
  //     send_tx = false;
  //   }
  //   MS_TEST_HELPER_AWAIT_EVENT(e);
  //   if (e.id == CAN_DATAGRAM_EVENT_TX) {
  //     send_tx = true;
  //   }
  //   can_datagram_process_event(&e);
  //   can_process_event(&e);
  // }

  // TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
  // TEST_ASSERT_TRUE_MESSAGE(cb_called, "Callback wasn't called");
  // TEST_ASSERT_EQUAL(0, s_rx_data_len);
}

// void test_datagram_completeness(void) {
//   TEST_ASSERT_OK(dispatcher_init(s_board_id));

//   CanDatagramTxConfig tx_config = {
//     .dgram_type = TEST_DATA_GRAM_ID,
//     .destination_nodes_len = 1,
//     .destination_nodes = &s_board_id,
//     .data_len = TEST_DATA_LEN,
//     .data = s_tx_data,
//     .tx_cmpl_cb = tx_cmpl_cb,
//   };
//   prv_send_tx_to_fifo(tx_config);
//   TEST_ASSERT_EQUAL(DATAGRAM_STATUS_TX_COMPLETE, can_datagram_get_status());

//   bool cb_called = false;
//   TEST_ASSERT_OK(
//       dispatcher_register_callback(TEST_DATA_GRAM_ID, prv_dispatch_check_cb, &cb_called));

//   Event e = { 0 };
//   prv_tx_from_fifo();
//   MS_TEST_HELPER_CAN_TX_RX(CAN_DATAGRAM_EVENT_TX, CAN_DATAGRAM_EVENT_RX);

//   bool send_tx = true;
//   while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {  // Loop until rx complete
//     if (send_tx) {
//       prv_tx_from_fifo();
//       send_tx = false;
//     }
//     MS_TEST_HELPER_AWAIT_EVENT(e);
//     if (e.id == CAN_DATAGRAM_EVENT_TX) {
//       send_tx = true;
//     }
//     can_datagram_process_event(&e);
//     can_process_event(&e);
//   }
//   TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
//   TEST_ASSERT_TRUE(cb_called);
//   TEST_ASSERT_EQUAL(TEST_DATA_LEN, s_rx_data_len);
//   for (int i = 0; i < s_rx_data_len; ++i) {
//     TEST_ASSERT_EQUAL(s_tx_data[i], s_rx_data[i]);  // test the data from dispatcher is complete
//   }
// }

// void test_noexistant_callback(void) {
//   // send a datagram id with no associated callback
//   TEST_ASSERT_OK(dispatcher_init(s_board_id));

//   CanDatagramTxConfig tx_config = {
//     .dgram_type = TEST_DATA_GRAM_ID,
//     .destination_nodes_len = 1,
//     .destination_nodes = &s_board_id,
//     .data_len = TEST_DATA_LEN,
//     .data = s_tx_data,
//     .tx_cmpl_cb = tx_cmpl_cb,
//   };
//   prv_send_tx_to_fifo(tx_config);
//   TEST_ASSERT_EQUAL(DATAGRAM_STATUS_TX_COMPLETE, can_datagram_get_status());

//   Event e = { 0 };
//   prv_tx_from_fifo();
//   MS_TEST_HELPER_CAN_TX_RX(CAN_DATAGRAM_EVENT_TX, CAN_DATAGRAM_EVENT_RX);

//   bool send_tx = true;
//   while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {  // Loop until rx complete
//     if (send_tx) {
//       prv_tx_from_fifo();
//       send_tx = false;
//     }
//     MS_TEST_HELPER_AWAIT_EVENT(e);
//     if (e.id == CAN_DATAGRAM_EVENT_TX) {
//       send_tx = true;
//     }
//     can_datagram_process_event(&e);
//     can_process_event(&e);
//   }
//   TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
// }

// void test_register_invalid_id(void) {
//   TEST_ASSERT_NOT_OK(dispatcher_register_callback(NUM_BOOTLOADER_DATAGRAMS, NULL, NULL));
// }
