#include "can_datagram.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "can.h"
#include "can_pack_impl.h"
#include "can_transmit.h"
#include "can_unpack_impl.h"
#include "delay.h"
#include "event_queue.h"
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
#define TEST_CAN_START_MSG_ID 2
#define TEST_CAN_DT_MSG_ID 1
#define TEST_CAN_BUFFER_SIZE 8

#define TEST_DST_SIZE_SHORT 16
#define TEST_DATA_SIZE_SHORT 16
#define NUM_SHORT_TEST_MSG 9

#define TEST_DST_SIZE_LONG 255
#define TEST_DATA_SIZE_LONG 2048
#define NUM_LONG_TEST_MSG 293

static CanStorage s_can_storage;

static uint8_t s_data[TEST_DATA_SIZE_SHORT] = 
{ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p' };
static uint8_t s_dst[TEST_DST_SIZE_SHORT] = 
{ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h','a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
static uint8_t s_data_long[TEST_DATA_SIZE_LONG] = { 0 };
static uint8_t s_dst_long[TEST_DST_SIZE_LONG] = { 0 };

static uint8_t data_lengths[] = { 1, 4, 1, 8, 2, 8 };
static int s_num_msg_rx;
static int s_num_tx;
static bool s_start_message_set;


typedef union test_datagram_msg {  // Should I include this in the library?
  uint8_t data_u8[8];
  uint64_t data_u64;
} test_datagram_msg;

static uint8_t s_short_test_data_index;
static uint64_t s_test_data_lookup[NUM_SHORT_TEST_MSG] = 
{ 1, 3868426920, 3, 16, 7523094288207667809, 7523094288207667809, 16, 7523094288207667809, 8101815670912281193};


void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
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

static void prv_mock_dt_tx(uint8_t dst_size, uint16_t data_size,
  uint8_t *dst_data, uint8_t *data_data) {
  test_datagram_msg msg = { 0 };
  size_t msg_len = 0;
  static int rx_bytes_sent;
  CanDatagram test_dt = {
    .protocol_version = 1,
    .crc = 0x12121212,
    .dt_type = 3,
    .destination_nodes_len = dst_size,
    .destination_nodes = dst_data,
    .data_len = data_size,
    .data = data_data,
  };
  switch (s_num_tx) {
    case 0:
      LOG_DEBUG("Protoc_V\n");
      msg.data_u8[0] = test_dt.protocol_version;
      msg_len = 1;
      s_num_tx++;
      break;
    case 1:
      LOG_DEBUG("CRC\n");
      msg.data_u64 = test_dt.crc;
      msg_len = 4;
      s_num_tx++;
      break;
    case 2:
      LOG_DEBUG("TYPE\n");
      msg.data_u8[0] = test_dt.dt_type;
      msg_len = 1;
      s_num_tx++;
      break;
    case 3:
      LOG_DEBUG("DST_LEN\n");
      msg.data_u8[0] = test_dt.destination_nodes_len;
      msg_len = 1;
      s_num_tx++;
      break;
    case 4: ;
      // LOG_DEBUG("DST\n");
      uint8_t dst_len = test_dt.destination_nodes_len;
      uint8_t dst_bytes_to_send = (dst_len - rx_bytes_sent < TEST_CAN_BUFFER_SIZE) ?
       (dst_len - rx_bytes_sent) : TEST_CAN_BUFFER_SIZE;
      for (int i = 0; i < dst_bytes_to_send; i++) {
        msg.data_u8[i] = test_dt.destination_nodes[rx_bytes_sent];
        rx_bytes_sent++;
      }
      if (rx_bytes_sent == dst_size) {
        s_num_tx++;
        rx_bytes_sent = 0;
      }
      msg_len = dst_bytes_to_send;
      break;
    case 5:
      memcpy(msg.data_u8, &test_dt.data_len, 2);
      s_num_tx++;
      msg_len = 2;
      break;
    case 6: ;
      //LOG_DEBUG("DATA\n");
      uint16_t data_len = test_dt.data_len;
      uint8_t data_bytes_to_send = (data_len - rx_bytes_sent < TEST_CAN_BUFFER_SIZE) ?
       (data_len - rx_bytes_sent) : TEST_CAN_BUFFER_SIZE;
      for (int i = 0; i < data_bytes_to_send; i++) {
        msg.data_u8[i] = test_dt.data[rx_bytes_sent];
        rx_bytes_sent++;
      }
      if (rx_bytes_sent == data_size) {
        s_num_tx++;
        rx_bytes_sent = 0;
      }
      msg_len = data_bytes_to_send;
      break;
    default:
      return;
  }
  CanMessage can_msg;
  can_pack_impl_u64(&can_msg, TEST_CAN_DEVICE_ID, TEST_CAN_DT_MSG_ID, msg_len, msg.data_u64);
  can_transmit(&can_msg, NULL);
}

static StatusCode prv_tx_callback(uint8_t *data, size_t len, bool start_message) {
    CanMessage msg;
    if (start_message) {
      can_pack_impl_u64(&msg, TEST_CAN_DEVICE_ID, TEST_CAN_START_MSG_ID, 0, 0);
    } else {
      test_datagram_msg msg_data = { 0 };
      memcpy(msg_data.data_u8, data, len);
      can_pack_impl_u64(&msg, TEST_CAN_DEVICE_ID, TEST_CAN_DT_MSG_ID, len, msg_data.data_u64);
    }
    can_transmit(&msg, NULL);
    return STATUS_CODE_OK;
}

static StatusCode prv_tx_init_rx_handler(const CanMessage *msg, void *context, CanAckStatus
*ack_reply) {
  LOG_DEBUG("TX init message received!\n");
  s_start_message_set = true;
  return STATUS_CODE_OK;
}

static StatusCode prv_rx_init_rx_handler(const CanMessage *msg, void *context, CanAckStatus
*ack_reply) {
  LOG_DEBUG("RX init message received!\n");
  can_datagram_rx(NULL, 0, true);
  s_start_message_set = true;
  return STATUS_CODE_OK;
}

static StatusCode prv_test_short_tx_rx_handler(const CanMessage *msg, void *context, CanAckStatus
*ack_reply) {
    s_num_msg_rx++;
    test_datagram_msg data = { 0 };
    can_unpack_impl_u64(msg, msg->dlc, &data.data_u64);
    TEST_ASSERT_EQUAL(s_test_data_lookup[s_short_test_data_index], data.data_u64);
    s_short_test_data_index++;
    return STATUS_CODE_OK;
}

static StatusCode prv_test_long_tx_rx_handler(const CanMessage *msg, void *context, CanAckStatus
*ack_reply) {
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

void test_can_datagram_tx(void) {
    prv_initialize_can();
    CanDatagramSettings settings = {
        .tx_cb = prv_tx_callback,
        .mode = CAN_DATAGRAM_MODE_TX,
        .dt_type = 3,
        .destination_nodes_len = TEST_DST_SIZE_SHORT,
        .destination_nodes = s_dst,
        .data_len = TEST_DATA_SIZE_SHORT,
        .data = s_data,
    };
    can_datagram_init(&settings);
    can_register_rx_handler(TEST_CAN_DT_MSG_ID, prv_test_short_tx_rx_handler, NULL);
    can_register_rx_handler(TEST_CAN_START_MSG_ID, prv_tx_init_rx_handler, NULL);
    can_datagram_start_tx(NULL, 0);

    Event e = { 0 };
    while(s_num_msg_rx < NUM_SHORT_TEST_MSG) { // Loop until num msg rx'd same as tx'd
        MS_TEST_HELPER_AWAIT_EVENT(e);
        can_datagram_process_event(&e);
        can_process_event(&e);
    }
    TEST_ASSERT_EQUAL(true, s_start_message_set);
    //MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void test_long_can_datagram_tx(void) {
    prv_initialize_can();
    CanDatagramSettings settings = {
        .tx_cb = prv_tx_callback,
        .mode = CAN_DATAGRAM_MODE_TX,
        .dt_type = 3,
        .destination_nodes_len = TEST_DST_SIZE_LONG,
        .destination_nodes = s_dst_long,
        .data_len = TEST_DATA_SIZE_LONG,
        .data = s_data_long,
    };
    can_datagram_init(&settings);
    can_register_rx_handler(TEST_CAN_DT_MSG_ID, prv_test_long_tx_rx_handler, NULL);
    can_datagram_start_tx(NULL, 0);

    Event e = { 0 };
    uint16_t count = 0;
    while(s_num_msg_rx < NUM_LONG_TEST_MSG) { // Loop until num msg rx'd same as tx'd
        MS_TEST_HELPER_AWAIT_EVENT(e);
        can_datagram_process_event(&e);
        can_process_event(&e);
    }
    //MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void test_can_datagram_rx(void) {
  uint8_t *rx_dst_buf = malloc(TEST_DST_SIZE_SHORT);
  uint8_t *rx_data_buf = malloc(TEST_DATA_SIZE_SHORT);
  prv_initialize_can();
  CanDatagramSettings settings = {
    .tx_cb = NULL,
    .mode = CAN_DATAGRAM_MODE_RX,
    .dt_type = 0,
    //.destination_nodes_len = TEST_DST_SIZE_SHORT,
    .destination_nodes = rx_dst_buf,
    //.data_len = TEST_DATA_SIZE_SHORT,
    .data = rx_data_buf,
  };

  can_datagram_init(&settings);
  can_register_rx_handler(TEST_CAN_DT_MSG_ID, prv_can_datagram_rx_handler, NULL);
  can_register_rx_handler(TEST_CAN_START_MSG_ID, prv_rx_init_rx_handler, NULL);

  // Send mock start message
  CanMessage msg = { 0 };
  can_pack_impl_empty(&msg, TEST_CAN_DEVICE_ID, TEST_CAN_START_MSG_ID);
  can_transmit(&msg, NULL);
  MS_TEST_HELPER_CAN_TX_RX(CAN_DATAGRAM_EVENT_TX, CAN_DATAGRAM_EVENT_RX);
  TEST_ASSERT_EQUAL(true, s_start_message_set);

  Event e = { 0 };
  bool send_tx = true;
  while (!can_datagram_complete()) {  // Loop until rx complete
    // Send txes one at a time -> can datagram will send 4 at a time
    // but client in real scenario does not have to process tx's as well as rx's
    if(send_tx) {
      prv_mock_dt_tx(TEST_DST_SIZE_SHORT, TEST_DATA_SIZE_SHORT, s_dst, s_data);
      send_tx = false;
    }
    MS_TEST_HELPER_AWAIT_EVENT(e);
    if(e.id == CAN_DATAGRAM_EVENT_TX) { 
      send_tx = true;
    }
    can_datagram_process_event(&e);
    can_process_event(&e);
  }

  CanDatagram *dt = can_datagram_get_datagram();
  TEST_ASSERT_EQUAL(1, dt->protocol_version);
  TEST_ASSERT_EQUAL(3, dt->dt_type);
  TEST_ASSERT_EQUAL(0x12121212, dt->crc);
  TEST_ASSERT_EQUAL(16, dt->destination_nodes_len);
  TEST_ASSERT_EQUAL(16, dt->data_len);
  // add data checking
  for(int i = 0; i < TEST_DST_SIZE_SHORT; i++) {
    TEST_ASSERT_EQUAL(s_dst[i], rx_dst_buf[i]);
  }
  for(uint16_t i = 0; i < TEST_DATA_SIZE_SHORT; i++) {
    TEST_ASSERT_EQUAL(s_data[i], rx_data_buf[i]);
  }
  free(rx_data_buf);
  free(rx_dst_buf);
}

void test_long_can_datagram_rx(void) {
  uint8_t *rx_dst_buf = malloc(TEST_DST_SIZE_LONG);
  uint8_t *rx_data_buf = malloc(TEST_DATA_SIZE_LONG);
  prv_initialize_can();

  CanDatagramSettings settings = {
    .tx_cb = NULL,
    .mode = CAN_DATAGRAM_MODE_RX,
    .dt_type = 0,
    //.destination_nodes_len = TEST_DST_SIZE_LONG,
    .destination_nodes = rx_dst_buf,
    //.data_len = TEST_DATA_SIZE_LONG,
    .data = rx_data_buf,
  };

  can_datagram_init(&settings);
  can_register_rx_handler(TEST_CAN_DT_MSG_ID, prv_can_datagram_rx_handler, NULL);
  can_register_rx_handler(TEST_CAN_START_MSG_ID, prv_rx_init_rx_handler, NULL);

  // Send mock start message
  CanMessage msg = { 0 };
  can_pack_impl_empty(&msg, TEST_CAN_DEVICE_ID, TEST_CAN_START_MSG_ID);
  can_transmit(&msg, NULL);
  MS_TEST_HELPER_CAN_TX_RX(CAN_DATAGRAM_EVENT_TX, CAN_DATAGRAM_EVENT_RX);
  TEST_ASSERT_EQUAL(true, s_start_message_set);

  Event e = { 0 };
  bool send_tx = true;
  while (!can_datagram_complete()) {  // Loop until rx complete
    if(send_tx) {
      prv_mock_dt_tx(TEST_DST_SIZE_LONG, TEST_DATA_SIZE_LONG, s_dst_long, s_data_long);
      send_tx = false;
    }
    MS_TEST_HELPER_AWAIT_EVENT(e);

    // Send txes one at a time -> can datagram will send 4 at a time
    // but client will not have to process tx's as well as rx's
    if(e.id == CAN_DATAGRAM_EVENT_TX) { 
      send_tx = true;
    }
    can_datagram_process_event(&e);
    can_process_event(&e);
  }
  CanDatagram *dt = can_datagram_get_datagram();
  TEST_ASSERT_EQUAL(1, dt->protocol_version);
  TEST_ASSERT_EQUAL(3, dt->dt_type);
  TEST_ASSERT_EQUAL(0x12121212, dt->crc);
  TEST_ASSERT_EQUAL(TEST_DST_SIZE_LONG, dt->destination_nodes_len);
  TEST_ASSERT_EQUAL(TEST_DATA_SIZE_LONG, dt->data_len);

  for(int i = 0; i < TEST_DST_SIZE_LONG; i++) {
    TEST_ASSERT_EQUAL(s_dst_long[i], rx_dst_buf[i]);
  }
  for(uint16_t i = 0; i < TEST_DATA_SIZE_LONG; i++) {
    TEST_ASSERT_EQUAL(s_data_long[i], rx_data_buf[i]);
  }
  free(rx_data_buf);
  free(rx_dst_buf);
}