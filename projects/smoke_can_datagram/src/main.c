#include "can_datagram.h"

#include <string.h>

#include "can.h"
#include "test_helpers.h"
#include "can_pack_impl.h"
#include "can_transmit.h"
#include "can_unpack_impl.h"
#include "delay.h"
#include "event_queue.h"
#include "interrupt.h"
#include "crc32.h"
#include "fifo.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"

#define TEST_CAN_DEVICE_ID 1
#define TEST_CAN_START_MSG_ID 1
#define TEST_CAN_DGRAM_MSG_ID 2 


// TEST INFORMATION - NOT RELEVANT TO IMPLEMNTATION
#define TEST_DST_SIZE 16
#define TEST_DATA_SIZE 16
#define TX_FIFO_SIZE 512
#define PROTOCOL_VERSION_SIZE_BYTES 1
#define CRC_SIZE_BYTES 4
#define DGRAM_TYPE_SIZE_BYTES 1
#define DEST_LEN_SIZE_BYTES 1
#define DATA_LEN_SIZE_BYTES 2
#define TEST_CAN_BUFFER_SIZE 8
#define MOCK_TX_PING_TIME_MS 50
static uint8_t s_tx_buffer[TX_FIFO_SIZE];
static Fifo s_tx_fifo;

typedef union test_datagram_msg { 
  uint8_t data_u8[8];
  uint64_t data_u64;
} test_datagram_msg;

static void prv_setup_tx_fifo(uint8_t dest_nodes_size, uint16_t data_size, uint8_t *dest_nodes,
                           uint8_t *data, uint32_t crc) {
  CanDatagram test_dgram = {
    .protocol_version = 1,
    .crc = crc,
    .dgram_type = '3', // This should be in every set of dst_node data
    .destination_nodes_len = dest_nodes_size,
    .destination_nodes = dest_nodes,
    .data_len = data_size,
    .data = data,
  };
  // Since we don't have to worry about space constraints, and can control tx rate, we can do this without a state machine
  fifo_init(&s_tx_fifo, s_tx_buffer);
  fifo_push_arr(&s_tx_fifo, &test_dgram.protocol_version, PROTOCOL_VERSION_SIZE_BYTES);
  fifo_push_arr(&s_tx_fifo, (uint8_t*)&test_dgram.crc, CRC_SIZE_BYTES);
  fifo_push_arr(&s_tx_fifo, &test_dgram.dgram_type, DGRAM_TYPE_SIZE_BYTES);
  fifo_push_arr(&s_tx_fifo, &test_dgram.destination_nodes_len, DEST_LEN_SIZE_BYTES);
  fifo_push_arr(&s_tx_fifo, test_dgram.destination_nodes, test_dgram.destination_nodes_len);
  fifo_push_arr(&s_tx_fifo, (uint8_t*)&test_dgram.data_len, DATA_LEN_SIZE_BYTES);
  fifo_push_arr(&s_tx_fifo, test_dgram.data, test_dgram.data_len);
  CanMessage msg = { 0 };
  //LOG_DEBUG("HEERE FROM ")
  can_pack_impl_empty(&msg, TEST_CAN_DEVICE_ID, TEST_CAN_START_MSG_ID);
  LOG_DEBUG("CAN_TRANSMIT_RX: %d\n", can_transmit(&msg, NULL));
}


// END OF TEST SPECIFIC INFORMATION

static CanStorage s_can_storage;

typedef enum {
  CAN_DATAGRAM_EVENT_RX = 0,
  CAN_DATAGRAM_EVENT_TX,
  CAN_DATAGRAM_EVENT_FAULT,
  NUM_CAN_DATAGRAM_EVENTS, // 3
} CanDatagramCanEvent;

typedef enum {
  DATAGRAM_EVENT_TX = NUM_CAN_DATAGRAM_EVENTS, //3
  DATAGRAM_EVENT_RX,
  DATAGRAM_EVENT_REPEAT,
  DATAGRAM_EVENT_ERROR,
  NUM_DATAGRAM_DIGEST_EVENTS,
} CanDatagramEvent;

typedef enum {
    MODE_TX = 0,
    MODE_RX,
} DgramMode;

static uint8_t s_rx_dst_buf[TEST_DST_SIZE];
static uint8_t s_rx_data_buf[TEST_DATA_SIZE];
static DgramMode s_dmode;

static CanDatagramRxConfig s_rx_config = {
.data = s_rx_dst_buf,
.destination_nodes = s_rx_data_buf,
.node_id = 'a',
};

static uint8_t s_tx_data[TEST_DATA_SIZE] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                                'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p' };
static uint8_t s_tx_dst[TEST_DST_SIZE] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                              'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };

static void prv_mock_dgram_tx(SoftTimerId timer_id, void *context) {
  CanMessage can_msg;
  test_datagram_msg msg = { 0 };
  size_t msg_len = fifo_size(&s_tx_fifo) < TEST_CAN_BUFFER_SIZE ? fifo_size(&s_tx_fifo) : TEST_CAN_BUFFER_SIZE;
  fifo_pop_arr(&s_tx_fifo, msg.data_u8, msg_len);
  can_pack_impl_u64(&can_msg, TEST_CAN_DEVICE_ID, TEST_CAN_DGRAM_MSG_ID, msg_len, msg.data_u64);
  can_transmit(&can_msg, NULL);
  soft_timer_start_millis(MOCK_TX_PING_TIME_MS, prv_mock_dgram_tx, NULL, NULL);
}

static StatusCode prv_tx_callback(uint8_t *data, size_t len, bool start_message);
static CanDatagramTxConfig s_tx_config = {
    .tx_cb = prv_tx_callback,
    .dgram_type = 3,
    .destination_nodes_len = TEST_DST_SIZE,
    .destination_nodes = s_tx_dst,
    .data_len = TEST_DATA_SIZE,
    .data = s_tx_data,
};

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

static StatusCode prv_tx_callback(uint8_t *data, size_t len, bool start_message) {
  //LOG_DEBUG("CALLED!\n");
  CanMessage msg;
  if (start_message) {
    can_pack_impl_u64(&msg, 2, 3, 0, 0);
  } else {
    test_datagram_msg msg_data = { 0 };
    memcpy(msg_data.data_u8, data, len);
    can_pack_impl_u64(&msg, 2, 3, len, msg_data.data_u64);
  }
  can_transmit(&msg, NULL);
  return STATUS_CODE_OK;
}

static void prv_handle_datagram_exit(int ret) {
    Event e = { 0 };
    if (ret == DATAGRAM_STATUS_TX_COMPLETE) {
        LOG_DEBUG("TX COMPLETE!\n");
        while (status_ok(event_process(&e))) {
        }
        can_datagram_start_listener(&s_rx_config);
        prv_setup_tx_fifo(TEST_DST_SIZE, TEST_DATA_SIZE, s_tx_dst,  s_tx_data, 0xf2bab4ac);
    } else if (ret == DATAGRAM_STATUS_RX_COMPLETE) {
        LOG_DEBUG("RX COMPLETE!\n");
        while (status_ok(event_process(&e))) {
        }
        s_dmode = MODE_RX;
        can_datagram_start_tx(&s_tx_config);
    } else if (ret == DATAGRAM_STATUS_ERROR) {
        LOG_DEBUG("ERROR %d!\n", ret);
    } else {
        LOG_DEBUG("Called!\n");
    }
}

static StatusCode prv_rx_handler(const CanMessage *msg, void *context,
                                               CanAckStatus *ack_reply) {
  // LOG_DEBUG("CALLED2!\n");
  test_datagram_msg data = { 0 };
  can_unpack_impl_u64(msg, msg->dlc, &data.data_u64);
  can_datagram_rx(data.data_u8, msg->dlc, false);

  return STATUS_CODE_OK;
}

static StatusCode prv_init_rx_handler(const CanMessage *msg, void *context,
                                         CanAckStatus *ack_reply) {
  LOG_DEBUG("RX init message received!\n");
  can_datagram_rx(NULL, 0, true);
  return STATUS_CODE_OK;
}

int main(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  crc32_init();
  prv_initialize_can();

  CanDatagramSettings settings = {
    .tx_event = DATAGRAM_EVENT_TX,
    .rx_event = DATAGRAM_EVENT_RX,
    .repeat_event = DATAGRAM_EVENT_REPEAT,
    .error_event = DATAGRAM_EVENT_ERROR,
  };
  can_datagram_init(&settings);
  can_register_rx_handler(TEST_CAN_DGRAM_MSG_ID, prv_rx_handler, NULL);
  can_register_rx_handler(TEST_CAN_START_MSG_ID, prv_init_rx_handler, NULL);

  Event e = { 0 };

//   can_datagram_start_listener(&s_rx_config);
//   prv_setup_tx_fifo(TEST_DST_SIZE, TEST_DATA_SIZE, s_tx_dst,  s_tx_data, 0xf2bab4ac);
//   CanMessage msg = { 0 };
//   can_pack_impl_empty(&msg, TEST_CAN_DEVICE_ID, TEST_CAN_START_MSG_ID);
//   can_transmit(&msg, NULL);
  soft_timer_start_millis(MOCK_TX_PING_TIME_MS, prv_mock_dgram_tx, NULL, NULL);
  can_datagram_start_tx(&s_tx_config);
  while(true) {
    while(event_process(&e) != STATUS_CODE_OK){}
    if(e.id == DATAGRAM_EVENT_RX) {
        LOG_DEBUG("RX EVENT!\n");
    }
    can_datagram_process_event(&e);
    can_process_event(&e);
    int ret = can_datagram_get_status();
    if(ret) {
        prv_handle_datagram_exit(ret);
    }
    delay_ms(10);
  }
}