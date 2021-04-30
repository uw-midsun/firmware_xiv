#include "can_datagram.h"

#include <stdint.h>
#include <stdio.h>

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
#define TEST_CAN_INIT_MSG_ID 1
#define TEST_CAN_DT_MSG_ID 2
#define TEST_DST_SIZE 16
#define TEST_DATA_SIZE 16
#define TEST_CAN_BUFFER_SIZE 8

#define NUM_TEST_MSG_TX 9

static CanStorage s_can_storage;

static uint8_t s_data[TEST_DATA_SIZE] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                          'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p' };
static uint8_t s_dst[TEST_DST_SIZE] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };

static uint8_t data_lengths[] = { 1, 4, 1, 8, 2, 8 };
static int s_num_msg_rx;

typedef union test_datagram_msg {  // Should I include this in the library?
  uint8_t data_u8[8];
  uint64_t data_u64;
} test_datagram_msg;

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
}

void teardown_test(void) {}

// static StatusCode prv_tx_callback(uint8_t *data, size_t len, bool start_message) {
//     if (start_message) {
//         return STATUS_CODE_OK;
//     }
//     CanMessage msg;
//     test_datagram_msg msg_data = { 0 };
//     for(uint8_t i = 0; i < len; i++) {
//         msg_data.data_u8[i] = data[i];
//     }
//     can_pack_impl_u64(&msg, TEST_CAN_DEVICE_ID, TEST_CAN_DT_MSG_ID, len, msg_data.data_u64);
//     can_transmit(&msg, NULL);
//     return STATUS_CODE_OK;
// }

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

// static StatusCode prv_test_tx_rx_handler(const CanMessage *msg, void *context, CanAckStatus
// *ack_reply) {
//     s_num_msg_rx++;
//     return STATUS_CODE_OK;
// }

static void prv_mock_dt_tx(void) {
  test_datagram_msg msg = { 0 };
  size_t msg_len = 0;
  static int rx_bytes_read;
  static int s_num_tx;
  CanDatagram test_dt = {
    .protocol_version = 1,
    .crc = 0x12121212,
    .dt_type = 0,
    .destination_nodes_len = TEST_DST_SIZE,
    .destination_nodes = s_dst,
    .data_len = TEST_DATA_SIZE,
    .data = s_data,
  };
  switch (s_num_tx) {
    // case 0:
    //     LOG_DEBUG("SENDING STARTUP MSG\n");
    //     msg.data_u64 = 0;
    //     CanMessage can_msg;
    //     can_pack_impl_u64(&can_msg, TEST_CAN_DEVICE_ID, TEST_CAN_INIT_MSG_ID, msg_len,
    //     msg.data_u64); can_transmit(&can_msg, NULL); s_num_tx++; return;
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
    case 4:
      LOG_DEBUG("DST\n");
      for (int i = 0; i < TEST_CAN_BUFFER_SIZE; i++) {
        msg.data_u8[i] = test_dt.destination_nodes[rx_bytes_read];
        rx_bytes_read++;
      }
      if (rx_bytes_read == TEST_DST_SIZE) {
        s_num_tx++;
        rx_bytes_read = 0;
      }
      msg_len = 8;
      break;
    case 5:
      LOG_DEBUG("DATA_LEN\n");
      msg.data_u8[0] = test_dt.data_len;
      s_num_tx++;
      msg_len = 1;
      break;
    case 6:
      LOG_DEBUG("DATA\n");
      for (int i = 0; i < TEST_CAN_BUFFER_SIZE; i++) {
        msg.data_u8[i] = test_dt.data[rx_bytes_read];
        rx_bytes_read++;
      }
      if (rx_bytes_read == TEST_DST_SIZE) {
        s_num_tx++;
      }
      msg_len = 8;
      break;
    default:
      return;
  }
  CanMessage can_msg;
  can_pack_impl_u64(&can_msg, TEST_CAN_DEVICE_ID, TEST_CAN_DT_MSG_ID, msg_len, msg.data_u64);
  can_transmit(&can_msg, NULL);
}

// static StatusCode prv_can_datagram_rx_init_handler(const CanMessage *msg, void *context,
// CanAckStatus *ack_reply) {
//     can_datagram_start();
//     return STATUS_CODE_OK;
// }

static StatusCode prv_can_datagram_rx_handler(const CanMessage *msg, void *context,
                                              CanAckStatus *ack_reply) {
  LOG_DEBUG("RX HANDLER CALLED\n");
  test_datagram_msg data = { 0 };
  can_unpack_impl_u64(msg, msg->dlc, &data.data_u64);
  // for(int i = 0; i < 8; i++) {
  //   LOG_DEBUG("RX DATA: %d\n", data.data_u8[i]);
  // }
  can_datagram_rx(data.data_u8, msg->dlc, false);
  return STATUS_CODE_OK;
}

// void test_can_datagram_tx(void) {
//     prv_initialize_can();
//     CanDatagramSettings settings = {
//         .tx_cb = prv_tx_callback,
//         .mode = CAN_DATAGRAM_MODE_TX,
//         .dt_type = 0,
//         .destination_nodes_len = TEST_DST_SIZE,
//         .destination_nodes = s_dst,
//         .data_len = TEST_DATA_SIZE,
//         .data = s_data,
//     };

//     can_datagram_init(&settings);
//     can_register_rx_handler(TEST_CAN_DT_MSG_ID, prv_test_tx_rx_handler, NULL);
//     can_datagram_start();

//     s_num_msg_rx = 0;
//     Event e = { 0 };
//     while(s_num_msg_rx < NUM_TEST_MSG_TX) { // Loop until num msg rx'd same as tx'd
//         MS_TEST_HELPER_AWAIT_EVENT(e);
//         can_datagram_process_event(&e);
//         can_process_event(&e);
//     }
// }

void test_can_datagram_rx(void) {
  uint8_t *rx_dst_buf = malloc(TEST_DST_SIZE);
  uint8_t *rx_data_buf = malloc(TEST_DATA_SIZE);
  prv_initialize_can();
  CanDatagramSettings settings = {
    .tx_cb = NULL,
    .mode = CAN_DATAGRAM_MODE_RX,
    .dt_type = 0,
    .destination_nodes_len = TEST_DST_SIZE,
    .destination_nodes = s_dst,
    .data_len = TEST_DATA_SIZE,
    .data = s_data,
  };

  can_datagram_init(&settings);
  can_register_rx_handler(TEST_CAN_DT_MSG_ID, prv_can_datagram_rx_handler, NULL);
  // can_register_rx_handler(TEST_CAN_INIT_MSG_ID, prv_can_datagram_rx_init_handler, NULL);
  prv_mock_dt_tx();
  can_datagram_start();
  Event e = { 0 };
  while (!can_datagram_complete()) {  // Loop until num msg rx'd same as tx'd
    prv_mock_dt_tx();
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
    can_process_event(&e);
  }
  free(rx_data_buf);
  free(rx_dst_buf);
}
