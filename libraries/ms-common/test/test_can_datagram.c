#include "can_datagram.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "can.h"
#include "can_pack_impl.h"
#include "can_transmit.h"
#include "can_unpack_impl.h"
#include "crc32.h"
#include "delay.h"
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
#include "test_helpers.h"
#include "unity.h"

#define TEST_CAN_DEVICE_ID 1
#define TEST_CAN_START_MSG_ID 3
#define TEST_CAN_DGRAM_MSG_ID_TX 1  // MSG ID USED TO TEST DGRAM TX
#define TEST_CAN_DGRAM_MSG_ID_RX 2  // MSG ID USED TO TEST DGRAM RX
#define TEST_CAN_BUFFER_SIZE 8

// Brought in from can_datagram.c
#define PROTOCOL_VERSION_SIZE_BYTES 1
#define CRC_SIZE_BYTES 4
#define DGRAM_TYPE_SIZE_BYTES 1
#define DEST_LEN_SIZE_BYTES 1
#define DATA_LEN_SIZE_BYTES 2
#define TX_FIFO_SIZE                                                                            \
  (PROTOCOL_VERSION_SIZE_BYTES + CRC_SIZE_BYTES + DGRAM_TYPE_SIZE_BYTES + DEST_LEN_SIZE_BYTES + \
   DGRAM_MAX_DEST_NODES_SIZE + DATA_LEN_SIZE_BYTES + DGRAM_MAX_DATA_SIZE)

#define TEST_DST_SIZE_SHORT 16
#define TEST_DATA_SIZE_SHORT 16
#define NUM_SHORT_TEST_MSG 6

#define TEST_DST_SIZE_LONG 255
#define TEST_DATA_SIZE_LONG 2048
#define NUM_LONG_TEST_MSG 289

#define RX_WATCHDOG_TIMEOUT_MS 1000

static CanStorage s_can_storage;

static uint8_t s_data[TEST_DATA_SIZE_SHORT] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                                'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p' };
static uint8_t s_dst[TEST_DST_SIZE_SHORT] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                              'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
static uint8_t s_data_long[TEST_DATA_SIZE_LONG] = { 0 };
static uint8_t s_dst_long[TEST_DST_SIZE_LONG] = { 0 };

static uint8_t data_lengths[] = { 1, 4, 1, 8, 2, 8 };
static int s_num_msg_rx;
static int s_num_tx;
static bool s_start_message_set;

static uint8_t s_tx_buffer[TX_FIFO_SIZE];
static Fifo s_tx_fifo;

typedef enum {
  CAN_DATAGRAM_EVENT_RX = 0,
  CAN_DATAGRAM_EVENT_TX,
  CAN_DATAGRAM_EVENT_FAULT,
  NUM_CAN_DATAGRAM_EVENTS,  // 3
} CanDatagramCanEvent;

typedef enum {
  DATAGRAM_EVENT_TX = NUM_CAN_DATAGRAM_EVENTS,  // 3
  DATAGRAM_EVENT_RX,
  DATAGRAM_EVENT_REPEAT,
  DATAGRAM_EVENT_ERROR,
  NUM_DATAGRAM_DIGEST_EVENTS,
} CanDatagramEvent;

typedef union test_datagram_msg {
  uint8_t data_u8[8];
  uint64_t data_u64;
} test_datagram_msg;

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  crc32_init();
}

void teardown_test(void) {
  Event e;
  while (status_ok(event_process(&e))) {
  }
  s_start_message_set = false;
  s_num_msg_rx = 0;
  s_num_tx = 0;
  s_num_msg_rx = 0;
}

static void prv_initialize_can() {
  CanSettings can_settings = {
    .device_id = TEST_CAN_DEVICE_ID,
    .loopback = true,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = CAN_DATAGRAM_EVENT_RX,
    .tx_event = CAN_DATAGRAM_EVENT_TX,
    .fault_event = CAN_DATAGRAM_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  can_init(&s_can_storage, &can_settings);
}

// Sets up a fifo of data structured the same way as can_datagram_tx to test rx
// Must be called before prv_mock_dgram_tx
static void prv_setup_tx_fifo(uint8_t dest_nodes_size, uint16_t data_size, uint8_t *dest_nodes,
                              uint8_t *data, uint32_t crc) {
  CanDatagram test_dgram = {
    .protocol_version = 1,
    .crc = crc,
    .dgram_type = 0,
    .destination_nodes_len = dest_nodes_size,
    .destination_nodes = dest_nodes,
    .data_len = data_size,
    .data = data,
  };
  // Since we don't have to worry about space constraints, and can control tx rate, we can do this
  // without a state machine
  fifo_init(&s_tx_fifo, s_tx_buffer);
  fifo_push_arr(&s_tx_fifo, &test_dgram.protocol_version, PROTOCOL_VERSION_SIZE_BYTES);
  fifo_push_arr(&s_tx_fifo, (uint8_t *)&test_dgram.crc, CRC_SIZE_BYTES);
  fifo_push_arr(&s_tx_fifo, &test_dgram.dgram_type, DGRAM_TYPE_SIZE_BYTES);
  fifo_push_arr(&s_tx_fifo, &test_dgram.destination_nodes_len, DEST_LEN_SIZE_BYTES);
  fifo_push_arr(&s_tx_fifo, test_dgram.destination_nodes, test_dgram.destination_nodes_len);
  fifo_push_arr(&s_tx_fifo, (uint8_t *)&test_dgram.data_len, DATA_LEN_SIZE_BYTES);
  fifo_push_arr(&s_tx_fifo, test_dgram.data, test_dgram.data_len);
}

static void prv_mock_dgram_tx(void) {
  CanMessage can_msg;
  test_datagram_msg msg = { 0 };
  size_t msg_len =
      fifo_size(&s_tx_fifo) < TEST_CAN_BUFFER_SIZE ? fifo_size(&s_tx_fifo) : TEST_CAN_BUFFER_SIZE;
  fifo_pop_arr(&s_tx_fifo, msg.data_u8, msg_len);
  can_pack_impl_u64(&can_msg, TEST_CAN_DEVICE_ID, TEST_CAN_DGRAM_MSG_ID_RX, msg_len, msg.data_u64);
  can_transmit(&can_msg, NULL);
}

static StatusCode prv_tx_callback(uint8_t *data, size_t len, bool start_message) {
  CanMessage msg;
  if (start_message) {
    can_pack_impl_u64(&msg, TEST_CAN_DEVICE_ID, TEST_CAN_START_MSG_ID, 0, 0);
  } else {
    test_datagram_msg msg_data = { 0 };
    memcpy(msg_data.data_u8, data, len);
    can_pack_impl_u64(&msg, TEST_CAN_DEVICE_ID, TEST_CAN_DGRAM_MSG_ID_TX, len, msg_data.data_u64);
  }
  can_transmit(&msg, NULL);
  return STATUS_CODE_OK;
}

static StatusCode prv_tx_callback_no_can(uint8_t *data, size_t len, bool start_message) {
  return STATUS_CODE_OK;
}

static StatusCode prv_tx_init_rx_handler(const CanMessage *msg, void *context,
                                         CanAckStatus *ack_reply) {
  LOG_DEBUG("TX init message received!\n");
  s_start_message_set = true;
  return STATUS_CODE_OK;
}

static StatusCode prv_rx_init_rx_handler(const CanMessage *msg, void *context,
                                         CanAckStatus *ack_reply) {
  LOG_DEBUG("RX init message received!\n");
  can_datagram_rx(NULL, 0, true);
  s_start_message_set = true;
  return STATUS_CODE_OK;
}

static StatusCode prv_test_short_tx_rx_handler(const CanMessage *msg, void *context,
                                               CanAckStatus *ack_reply) {
  s_num_msg_rx++;
  test_datagram_msg data = { 0 };
  can_unpack_impl_u64(msg, msg->dlc, &data.data_u64);
  return STATUS_CODE_OK;
}

static StatusCode prv_test_long_tx_rx_handler(const CanMessage *msg, void *context,
                                              CanAckStatus *ack_reply) {
  s_num_msg_rx++;
  test_datagram_msg data = { 0 };
  can_unpack_impl_u64(msg, msg->dlc, &data.data_u64);
  return STATUS_CODE_OK;
}

static StatusCode prv_can_datagram_rx_handler(const CanMessage *msg, void *context,
                                              CanAckStatus *ack_reply) {
  test_datagram_msg data = { 0 };
  can_unpack_impl_u64(msg, msg->dlc, &data.data_u64);
  can_datagram_rx(data.data_u8, msg->dlc, false);

  return STATUS_CODE_OK;
}

static void prv_datagram_tx_cmpl(void) {
  LOG_DEBUG("TX_COMPLETE!\n");
}

static void prv_datagram_rx_cmpl(void) {
  LOG_DEBUG("RX_COMPLETE!\n");
}

static void prv_datagram_error(void) {
  LOG_DEBUG("ERROR!\n");
}

void test_can_datagram_tx(void) {
  prv_initialize_can();
  CanDatagramSettings settings = {
    .tx_event = DATAGRAM_EVENT_TX,
    .rx_event = DATAGRAM_EVENT_RX,
    .repeat_event = DATAGRAM_EVENT_REPEAT,
    .error_event = DATAGRAM_EVENT_ERROR,
  };
  can_datagram_init(&settings);
  can_register_rx_handler(TEST_CAN_DGRAM_MSG_ID_TX, prv_test_short_tx_rx_handler, NULL);
  can_register_rx_handler(TEST_CAN_START_MSG_ID, prv_tx_init_rx_handler, NULL);

  CanDatagramTxConfig tx_config = {
    .tx_cb = prv_tx_callback,
    .dgram_type = 3,
    .destination_nodes_len = TEST_DST_SIZE_SHORT,
    .destination_nodes = s_dst,
    .data_len = TEST_DATA_SIZE_SHORT,
    .data = s_data,
    .tx_cmpl_cb = prv_datagram_tx_cmpl,
  };
  can_datagram_start_tx(&tx_config);

  Event e = { 0 };
  while (can_datagram_get_status() ==
         DATAGRAM_STATUS_ACTIVE) {  // Loop until num msg rx'd same as tx'd
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
    can_process_event(&e);
  }
  TEST_ASSERT_EQUAL(NUM_SHORT_TEST_MSG, s_num_msg_rx);
  TEST_ASSERT_EQUAL(true, s_start_message_set);
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_TX_COMPLETE, can_datagram_get_status());
}

void test_can_datagram_rx(void) {
  uint8_t rx_dst_buf[TEST_DST_SIZE_SHORT];
  uint8_t rx_data_buf[TEST_DATA_SIZE_SHORT];
  prv_initialize_can();
  prv_setup_tx_fifo(TEST_DST_SIZE_SHORT, TEST_DATA_SIZE_SHORT, s_dst, s_data, 0x910d5058);

  CanDatagramSettings settings = {
    .tx_event = DATAGRAM_EVENT_TX,
    .rx_event = DATAGRAM_EVENT_RX,
    .repeat_event = DATAGRAM_EVENT_REPEAT,
    .error_event = DATAGRAM_EVENT_ERROR,
  };
  can_datagram_init(&settings);
  can_register_rx_handler(TEST_CAN_DGRAM_MSG_ID_RX, prv_can_datagram_rx_handler, NULL);
  can_register_rx_handler(TEST_CAN_START_MSG_ID, prv_rx_init_rx_handler, NULL);

  CanDatagramRxConfig rx_config = {
    .data = rx_data_buf,
    .destination_nodes = rx_dst_buf,
    .node_id = 'a',
    .rx_cmpl_cb = prv_datagram_rx_cmpl,
  };
  can_datagram_start_listener(&rx_config);

  // Send mock start message
  CanMessage msg = { 0 };
  can_pack_impl_empty(&msg, TEST_CAN_DEVICE_ID, TEST_CAN_START_MSG_ID);
  can_transmit(&msg, NULL);
  MS_TEST_HELPER_CAN_TX_RX(CAN_DATAGRAM_EVENT_TX, CAN_DATAGRAM_EVENT_RX);
  TEST_ASSERT_EQUAL(true, s_start_message_set);

  Event e = { 0 };
  bool send_tx = true;
  while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {  // Loop until rx complete
    // Send txes one at a time -> can datagram will send 4 at a time
    // but client in real scenario does not have to process tx's as well as rx's
    if (send_tx) {
      prv_mock_dgram_tx();
      send_tx = false;
    }
    MS_TEST_HELPER_AWAIT_EVENT(e);
    if (e.id == CAN_DATAGRAM_EVENT_TX) {
      send_tx = true;
    }
    can_datagram_process_event(&e);
    can_process_event(&e);
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());

  // Verify relevant data set in rx config
  TEST_ASSERT_EQUAL(0x910d5058, rx_config.crc);
  TEST_ASSERT_EQUAL(16, rx_config.destination_nodes_len);
  TEST_ASSERT_EQUAL(16, rx_config.data_len);
  // add data checking
  for (int i = 0; i < TEST_DST_SIZE_SHORT; i++) {
    TEST_ASSERT_EQUAL(s_dst[i], rx_dst_buf[i]);
  }
  for (uint16_t i = 0; i < TEST_DATA_SIZE_SHORT; i++) {
    TEST_ASSERT_EQUAL(s_data[i], rx_data_buf[i]);
  }
}

void test_long_can_datagram_tx(void) {
  prv_initialize_can();

  CanDatagramSettings settings = {
    .tx_event = DATAGRAM_EVENT_TX,
    .rx_event = DATAGRAM_EVENT_RX,
    .repeat_event = DATAGRAM_EVENT_REPEAT,
    .error_event = DATAGRAM_EVENT_ERROR,
  };
  can_datagram_init(&settings);
  can_register_rx_handler(TEST_CAN_DGRAM_MSG_ID_TX, prv_test_long_tx_rx_handler, NULL);
  can_register_rx_handler(TEST_CAN_START_MSG_ID, prv_tx_init_rx_handler, NULL);

  CanDatagramTxConfig tx_config = {
    .tx_cb = prv_tx_callback,
    .dgram_type = 3,
    .destination_nodes_len = TEST_DST_SIZE_LONG,
    .destination_nodes = s_dst_long,
    .data_len = TEST_DATA_SIZE_LONG,
    .data = s_data_long,
  };
  can_datagram_start_tx(&tx_config);

  Event e = { 0 };
  uint16_t count = 0;
  while (can_datagram_get_status() ==
         DATAGRAM_STATUS_ACTIVE) {  // Loop until num msg rx'd same as tx'd
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
    can_process_event(&e);
  }
  TEST_ASSERT_EQUAL(NUM_LONG_TEST_MSG, s_num_msg_rx);
  TEST_ASSERT_EQUAL(true, s_start_message_set);
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_TX_COMPLETE, can_datagram_get_status());
}

void test_long_can_datagram_rx(void) {
  uint8_t rx_dst_buf[TEST_DST_SIZE_LONG];
  uint8_t rx_data_buf[TEST_DATA_SIZE_LONG];
  prv_initialize_can();
  prv_setup_tx_fifo(TEST_DST_SIZE_LONG, TEST_DATA_SIZE_LONG, s_dst_long, s_data_long, 0x7f810ceb);

  CanDatagramSettings settings = {
    .tx_event = DATAGRAM_EVENT_TX,
    .rx_event = DATAGRAM_EVENT_RX,
    .repeat_event = DATAGRAM_EVENT_REPEAT,
    .error_event = DATAGRAM_EVENT_ERROR,
  };
  can_datagram_init(&settings);
  can_register_rx_handler(TEST_CAN_DGRAM_MSG_ID_RX, prv_can_datagram_rx_handler, NULL);
  can_register_rx_handler(TEST_CAN_START_MSG_ID, prv_rx_init_rx_handler, NULL);

  CanDatagramRxConfig rx_config = {
    .destination_nodes = rx_dst_buf,
    .data = rx_data_buf,
    .node_id = 0,
  };
  can_datagram_start_listener(&rx_config);

  // Send mock start message
  CanMessage msg = { 0 };
  can_pack_impl_empty(&msg, TEST_CAN_DEVICE_ID, TEST_CAN_START_MSG_ID);
  can_transmit(&msg, NULL);
  MS_TEST_HELPER_CAN_TX_RX(CAN_DATAGRAM_EVENT_TX, CAN_DATAGRAM_EVENT_RX);
  TEST_ASSERT_EQUAL(true, s_start_message_set);

  Event e = { 0 };
  bool send_tx = true;
  while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {  // Loop until rx complete
    if (send_tx) {
      prv_mock_dgram_tx();
      send_tx = false;
    }
    MS_TEST_HELPER_AWAIT_EVENT(e);
    // Send txes one at a time -> can datagram will send 4 at a time
    // but client will not have to process tx's as well as rx's
    if (e.id == CAN_DATAGRAM_EVENT_TX) {
      send_tx = true;
    }
    can_datagram_process_event(&e);
    can_process_event(&e);
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());

  TEST_ASSERT_EQUAL(0x7f810ceb, rx_config.crc);
  TEST_ASSERT_EQUAL(255, rx_config.destination_nodes_len);
  TEST_ASSERT_EQUAL(2048, rx_config.data_len);

  for (int i = 0; i < TEST_DST_SIZE_LONG; i++) {
    TEST_ASSERT_EQUAL(s_dst_long[i], rx_dst_buf[i]);
  }
  for (uint16_t i = 0; i < TEST_DATA_SIZE_LONG; i++) {
    TEST_ASSERT_EQUAL(s_data_long[i], rx_data_buf[i]);
  }
}

void test_start_msg_not_sent(void) {
  uint8_t rx_dst_buf[TEST_DST_SIZE_SHORT];
  uint8_t rx_data_buf[TEST_DATA_SIZE_SHORT];
  prv_initialize_can();
  prv_setup_tx_fifo(TEST_DST_SIZE_SHORT, TEST_DATA_SIZE_SHORT, s_dst, s_data, 0x1c36e81f);

  CanDatagramSettings settings = {
    .tx_event = DATAGRAM_EVENT_TX,
    .rx_event = DATAGRAM_EVENT_RX,
    .repeat_event = DATAGRAM_EVENT_REPEAT,
    .error_event = DATAGRAM_EVENT_ERROR,
  };
  can_datagram_init(&settings);

  CanDatagramRxConfig rx_config = {
    .destination_nodes = rx_dst_buf,
    .data = rx_data_buf,
    .node_id = 'a',
  };
  can_datagram_start_listener(&rx_config);

  can_register_rx_handler(TEST_CAN_DGRAM_MSG_ID_RX, prv_can_datagram_rx_handler, NULL);

  Event e = { 0 };
  // send a few non-start tx messages
  bool send_tx = true;
  for (int i = 0; i < 8; i++) {
    if (send_tx) {
      prv_mock_dgram_tx();
      send_tx = false;
    }
    MS_TEST_HELPER_AWAIT_EVENT(e);
    if (e.id == CAN_DATAGRAM_EVENT_TX) {
      send_tx = true;
    }
    can_datagram_process_event(&e);
    can_process_event(&e);
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_IDLE, can_datagram_get_status());
}

void test_rx_timeout(void) {
  uint8_t rx_dst_buf[TEST_DST_SIZE_SHORT];
  uint8_t rx_data_buf[TEST_DATA_SIZE_SHORT];
  prv_initialize_can();
  prv_setup_tx_fifo(TEST_DST_SIZE_SHORT, TEST_DATA_SIZE_SHORT, s_dst, s_data, 0x1c36e81f);

  CanDatagramSettings settings = {
    .tx_event = DATAGRAM_EVENT_TX,
    .rx_event = DATAGRAM_EVENT_RX,
    .repeat_event = DATAGRAM_EVENT_REPEAT,
    .error_event = DATAGRAM_EVENT_ERROR,
    .error_cb = prv_datagram_error,
  };
  can_datagram_init(&settings);
  can_register_rx_handler(TEST_CAN_DGRAM_MSG_ID_RX, prv_can_datagram_rx_handler, NULL);
  can_register_rx_handler(TEST_CAN_START_MSG_ID, prv_rx_init_rx_handler, NULL);

  CanDatagramRxConfig rx_config = {
    .destination_nodes = rx_dst_buf,
    .data = rx_data_buf,
    .node_id = 'a',
  };
  can_datagram_start_listener(&rx_config);

  // Send mock start message
  CanMessage msg = { 0 };
  can_pack_impl_empty(&msg, TEST_CAN_DEVICE_ID, TEST_CAN_START_MSG_ID);
  can_transmit(&msg, NULL);
  MS_TEST_HELPER_CAN_TX_RX(CAN_DATAGRAM_EVENT_TX, CAN_DATAGRAM_EVENT_RX);
  TEST_ASSERT_EQUAL(true, s_start_message_set);
  delay_ms(RX_WATCHDOG_TIMEOUT_MS + 5);
  Event e = { 0 };
  while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_process_event(&e);
    can_datagram_process_event(&e);
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_ERROR, can_datagram_get_status());
}

// Verify that Rx Message can be sent before and after Tx
// Since processing real CAN messages adds a lot of complexity to tests,
// will no longer be used
void test_rx_tx_rx() {
  uint8_t rx_dst_buf[TEST_DST_SIZE_SHORT];
  uint8_t rx_data_buf[TEST_DATA_SIZE_SHORT];
  prv_setup_tx_fifo(TEST_DST_SIZE_SHORT, 0, s_dst, NULL, 0x7e4a03ed);
  CanMessage msg = { 0 };
  Event e = { 0 };

  CanDatagramSettings settings = {
    .tx_event = DATAGRAM_EVENT_TX,
    .rx_event = DATAGRAM_EVENT_RX,
    .repeat_event = DATAGRAM_EVENT_REPEAT,
    .error_event = DATAGRAM_EVENT_ERROR,
  };
  can_datagram_init(&settings);

  CanDatagramRxConfig rx_config = {
    .data = rx_data_buf,
    .destination_nodes = rx_dst_buf,
    .node_id = 'a',
  };

  CanDatagramTxConfig tx_config = {
    .tx_cb = prv_tx_callback_no_can,
    .dgram_type = 3,
    .destination_nodes_len = TEST_DST_SIZE_SHORT,
    .destination_nodes = s_dst,
    .data_len = TEST_DATA_SIZE_SHORT,
    .data = s_data,
  };

  // Start Datagram TX
  can_datagram_start_tx(&tx_config);
  while (can_datagram_get_status() ==
         DATAGRAM_STATUS_ACTIVE) {  // Loop until num msg rx'd same as tx'd
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_TX_COMPLETE, can_datagram_get_status());

  // Start RX run
  can_datagram_start_listener(&rx_config);
  // Rcv mock start message
  can_datagram_rx(NULL, 0, true);
  uint8_t msg_buffer[TEST_CAN_BUFFER_SIZE];
  while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {  // Loop until rx complete
    size_t msg_len =
        fifo_size(&s_tx_fifo) < TEST_CAN_BUFFER_SIZE ? fifo_size(&s_tx_fifo) : TEST_CAN_BUFFER_SIZE;
    fifo_pop_arr(&s_tx_fifo, msg_buffer, msg_len);
    can_datagram_rx(msg_buffer, msg_len, false);
    if (event_process(&e) == STATUS_CODE_OK) {
      can_datagram_process_event(&e);
    }
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());

  // Start Datagram TX
  can_datagram_start_tx(&tx_config);
  while (can_datagram_get_status() ==
         DATAGRAM_STATUS_ACTIVE) {  // Loop until num msg rx'd same as tx'd
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_TX_COMPLETE, can_datagram_get_status());
}

// Test if current node not in message, it is ignored and state returns to idle
// and can resume execution
void test_soft_error_rx_successful(void) {
  uint8_t rx_dst_buf[TEST_DST_SIZE_SHORT];
  uint8_t rx_data_buf[TEST_DATA_SIZE_SHORT];
  prv_initialize_can();
  // prv_setup_tx_fifo(TEST_DST_SIZE_SHORT, TEST_DATA_SIZE_SHORT, s_dst, s_data, 0xf2bab4ac); //
  // this line was never used...

  CanDatagramSettings settings = {
    .tx_event = DATAGRAM_EVENT_TX,
    .rx_event = DATAGRAM_EVENT_RX,
    .repeat_event = DATAGRAM_EVENT_REPEAT,
    .error_event = DATAGRAM_EVENT_ERROR,
  };
  can_datagram_init(&settings);

  CanDatagramRxConfig rx_config = {
    .data = rx_data_buf,
    .destination_nodes = rx_dst_buf,
    .node_id = 'a',  // Not a member of the data
  };

  uint8_t test_node_buf[TEST_DST_SIZE_SHORT] = { 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
                                                 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q' };
  prv_setup_tx_fifo(TEST_DST_SIZE_SHORT, 0, test_node_buf, NULL, 0x8f4d35c1);
  // Start RX run
  can_datagram_start_listener(&rx_config);
  // Rcv mock start message
  can_datagram_rx(NULL, 0, true);
  Event e = { 0 };
  // Set timer to start a new message mock tx
  // SoftTimerId test_soft;
  uint8_t msg_buffer[TEST_CAN_BUFFER_SIZE];
  for (int i = 0; i < 25; i++) {
    size_t msg_len =
        fifo_size(&s_tx_fifo) < TEST_CAN_BUFFER_SIZE ? fifo_size(&s_tx_fifo) : TEST_CAN_BUFFER_SIZE;
    if (msg_len) {
      fifo_pop_arr(&s_tx_fifo, msg_buffer, msg_len);
      can_datagram_rx(msg_buffer, msg_len, false);
    }
    if (event_process(&e) == STATUS_CODE_OK) {
      can_datagram_process_event(&e);
    }
  }
  prv_setup_tx_fifo(TEST_DST_SIZE_SHORT, 0, s_dst, NULL, 0x7e4a03ed);
  can_datagram_rx(NULL, 0, true);
  while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {  // Loop until rx complete
    size_t msg_len =
        fifo_size(&s_tx_fifo) < TEST_CAN_BUFFER_SIZE ? fifo_size(&s_tx_fifo) : TEST_CAN_BUFFER_SIZE;
    if (msg_len) {
      fifo_pop_arr(&s_tx_fifo, msg_buffer, msg_len);
      can_datagram_rx(msg_buffer, msg_len, false);
    }
    if (event_process(&e) == STATUS_CODE_OK) {
      can_datagram_process_event(&e);
    }
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
}

void test_tx_rx_tx() {
  uint8_t rx_dst_buf[TEST_DST_SIZE_SHORT];
  uint8_t rx_data_buf[TEST_DATA_SIZE_SHORT];
  prv_setup_tx_fifo(TEST_DST_SIZE_SHORT, 0, s_dst, NULL, 0x7e4a03ed);
  CanMessage msg = { 0 };
  Event e = { 0 };

  CanDatagramSettings settings = {
    .tx_event = DATAGRAM_EVENT_TX,
    .rx_event = DATAGRAM_EVENT_RX,
    .repeat_event = DATAGRAM_EVENT_REPEAT,
    .error_event = DATAGRAM_EVENT_ERROR,
  };
  can_datagram_init(&settings);

  CanDatagramRxConfig rx_config = {
    .data = rx_data_buf,
    .destination_nodes = rx_dst_buf,
    .node_id = 'a',
  };

  CanDatagramTxConfig tx_config = {
    .tx_cb = prv_tx_callback_no_can,
    .dgram_type = 3,
    .destination_nodes_len = TEST_DST_SIZE_SHORT,
    .destination_nodes = s_dst,
    .data_len = TEST_DATA_SIZE_SHORT,
    .data = s_data,
  };

  // Start RX run
  can_datagram_start_listener(&rx_config);
  // Rcv mock start message
  can_datagram_rx(NULL, 0, true);
  uint8_t msg_buffer[TEST_CAN_BUFFER_SIZE];
  while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {  // Loop until rx complete
    size_t msg_len =
        fifo_size(&s_tx_fifo) < TEST_CAN_BUFFER_SIZE ? fifo_size(&s_tx_fifo) : TEST_CAN_BUFFER_SIZE;
    fifo_pop_arr(&s_tx_fifo, msg_buffer, msg_len);
    can_datagram_rx(msg_buffer, msg_len, false);
    if (event_process(&e) == STATUS_CODE_OK) {
      can_datagram_process_event(&e);
    }
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());

  // Start Datagram TX
  can_datagram_start_tx(&tx_config);
  while (can_datagram_get_status() ==
         DATAGRAM_STATUS_ACTIVE) {  // Loop until num msg rx'd same as tx'd
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_TX_COMPLETE, can_datagram_get_status());

  // Start RX run
  prv_setup_tx_fifo(TEST_DST_SIZE_SHORT, 0, s_dst, NULL, 0x7e4a03ed);
  can_datagram_start_listener(&rx_config);
  // Rcv mock start message
  can_datagram_rx(NULL, 0, true);
  while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {  // Loop until rx complete
    size_t msg_len =
        fifo_size(&s_tx_fifo) < TEST_CAN_BUFFER_SIZE ? fifo_size(&s_tx_fifo) : TEST_CAN_BUFFER_SIZE;
    fifo_pop_arr(&s_tx_fifo, msg_buffer, msg_len);
    can_datagram_rx(msg_buffer, msg_len, false);
    if (event_process(&e) == STATUS_CODE_OK) {
      can_datagram_process_event(&e);
    }
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
}

// Test node 0 is a part of destination node ids sent
// every node_id should receive the datagram
void test_id_0_tx(void) {
  uint8_t rx_dst_buf[TEST_DST_SIZE_SHORT];
  uint8_t rx_data_buf[TEST_DATA_SIZE_SHORT];
  prv_initialize_can();

  CanDatagramSettings settings = {
    .tx_event = DATAGRAM_EVENT_TX,
    .rx_event = DATAGRAM_EVENT_RX,
    .repeat_event = DATAGRAM_EVENT_REPEAT,
    .error_event = DATAGRAM_EVENT_ERROR,
  };
  can_datagram_init(&settings);

  CanDatagramRxConfig rx_config = {
    .data = rx_data_buf,
    .destination_nodes = rx_dst_buf,
    .node_id = 'a',
  };

  uint8_t test_node_buf[1] = { 0 };

  prv_setup_tx_fifo(1, TEST_DATA_SIZE_SHORT, test_node_buf, s_data, 0xdd119380);
  // Start RX run
  can_datagram_start_listener(&rx_config);
  // Rcv mock start message
  can_datagram_rx(NULL, 0, true);
  Event e = { 0 };
  // Set timer to start a new message mock tx
  // SoftTimerId test_soft;
  uint8_t msg_buffer[TEST_CAN_BUFFER_SIZE];
  for (int i = 0; i < 25; i++) {
    size_t msg_len =
        fifo_size(&s_tx_fifo) < TEST_CAN_BUFFER_SIZE ? fifo_size(&s_tx_fifo) : TEST_CAN_BUFFER_SIZE;
    if (msg_len) {
      fifo_pop_arr(&s_tx_fifo, msg_buffer, msg_len);
      can_datagram_rx(msg_buffer, msg_len, false);
    }
    if (event_process(&e) == STATUS_CODE_OK) {
      can_datagram_process_event(&e);
    }
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
  // Verify relevant data set in rx config
  TEST_ASSERT_EQUAL(0xdd119380, rx_config.crc);
  TEST_ASSERT_EQUAL(1, rx_config.destination_nodes_len);
  TEST_ASSERT_EQUAL(TEST_DATA_SIZE_SHORT, rx_config.data_len);
  // add data checking
  TEST_ASSERT_EQUAL(0, rx_dst_buf[0]);
  for (uint16_t i = 0; i < TEST_DATA_SIZE_SHORT; i++) {
    TEST_ASSERT_EQUAL(s_data[i], rx_data_buf[i]);
  }
}

// Test node 0 rxes every datagram
void test_id_0_rx(void) {
  uint8_t rx_dst_buf[TEST_DST_SIZE_SHORT];
  uint8_t rx_data_buf[TEST_DATA_SIZE_SHORT];
  prv_initialize_can();

  CanDatagramSettings settings = {
    .tx_event = DATAGRAM_EVENT_TX,
    .rx_event = DATAGRAM_EVENT_RX,
    .repeat_event = DATAGRAM_EVENT_REPEAT,
    .error_event = DATAGRAM_EVENT_ERROR,
  };
  can_datagram_init(&settings);

  CanDatagramRxConfig rx_config = {
    .data = rx_data_buf,
    .destination_nodes = rx_dst_buf,
    .node_id = 0,
  };

  prv_setup_tx_fifo(TEST_DST_SIZE_SHORT, TEST_DATA_SIZE_SHORT, s_dst, s_data, 0x910d5058);
  // Start RX run
  can_datagram_start_listener(&rx_config);
  // Rcv mock start message
  can_datagram_rx(NULL, 0, true);
  Event e = { 0 };
  // Set timer to start a new message mock tx
  // SoftTimerId test_soft;
  uint8_t msg_buffer[TEST_CAN_BUFFER_SIZE];
  for (int i = 0; i < 25; i++) {
    size_t msg_len =
        fifo_size(&s_tx_fifo) < TEST_CAN_BUFFER_SIZE ? fifo_size(&s_tx_fifo) : TEST_CAN_BUFFER_SIZE;
    if (msg_len) {
      fifo_pop_arr(&s_tx_fifo, msg_buffer, msg_len);
      can_datagram_rx(msg_buffer, msg_len, false);
    }
    if (event_process(&e) == STATUS_CODE_OK) {
      can_datagram_process_event(&e);
    }
  }
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
  // Verify relevant data set in rx config
  TEST_ASSERT_EQUAL(0x910d5058, rx_config.crc);
  TEST_ASSERT_EQUAL(TEST_DST_SIZE_SHORT, rx_config.destination_nodes_len);  // 16
  TEST_ASSERT_EQUAL(TEST_DATA_SIZE_SHORT, rx_config.data_len);              // 16
  // add data checking
  for (uint16_t i = 0; i < TEST_DST_SIZE_SHORT; i++) {
    TEST_ASSERT_EQUAL(s_dst[i], rx_dst_buf[i]);
  }
  for (uint16_t i = 0; i < TEST_DATA_SIZE_SHORT; i++) {
    TEST_ASSERT_EQUAL(s_data[i], rx_data_buf[i]);
  }
}
