#include "can_datagram.h"

#include <stdint.h>
#include <stdio.h>

#include "status.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "ms_test_helpers.h"
#include "unity.h"
#include "log.h"
#include "event_queue.h"
#include "can.h"
#include "can_pack_impl.h"
#include "can_transmit.h"
#include "can_unpack_impl.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"

#define TEST_CAN_DEVICE_ID 1
#define TEST_CAN_INIT_MSG_ID 1
#define TEST_CAN_DT_MSG_ID 2
#define TEST_DST_SIZE 8
#define TEST_DATA_SIZE 16

static CanStorage s_can_storage;

static uint8_t s_data[16] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};
static uint8_t s_dst[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

static uint8_t data_lengths[] = { 1, 4, 1, 8, 2, 8};
static uint8_t num_msgs_to_tx_rx;

void setup_test(void) {
    event_queue_init();
    interrupt_init();
    soft_timer_init();
}

typedef union test_datagram_msg { // Should I include this in the library?
    uint8_t data_u8[8];
    uint64_t data_u64;
} test_datagram_msg;


void teardown_test(void) {}

static StatusCode prv_tx_callback(uint8_t *data, size_t len, bool start_message) {
    if (start_message) {
        return STATUS_CODE_OK;
    }
    LOG_DEBUG("TX CALLBACK %ld\n", len);
    /*CanMessage msg;
    test_datagram_msg msg_data = { 0 };
    for(uint8_t i = 0; i < len; i++) {
        msg_data.data_u8[i] = data[i];
    }
    can_pack_impl_u64(&msg, TEST_CAN_DEVICE_ID, TEST_CAN_DT_MSG_ID, len, msg_data.data_u64);
    can_transmit(&msg, NULL);
    MS_TEST_HELPER_CAN_TX_RX(CAN_DATAGRAM_EVENT_TX, CAN_DATAGRAM_EVENT_RX);
    num_msgs_to_tx_rx++;*/

    return STATUS_CODE_OK;  
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

static StatusCode prv_can_datagram_rx_handler(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
    LOG_DEBUG("RX_HANDLER CALLED %ld!\n", msg->dlc);
    test_datagram_msg data = { 0 };

    can_unpack_impl_u64(msg, msg->dlc, &data.data_u64);
    /*for(uint8_t i = 0; i < msg->dlc; i++) {
        uint8_t chr = data.data_u8[i];
        LOG_DEBUG("Data %d = %c\n", i, chr);
    }*/

    return STATUS_CODE_OK;
}

void test_can_datagram_tx(void) {
    prv_initialize_can();
    CanDatagramSettings settings = {
        .tx_cb = prv_tx_callback,
        .mode = CAN_DATAGRAM_MODE_TX,
    };

    can_datagram_init(&settings);
    can_datagram_set_address_buffer(s_dst, TEST_DST_SIZE);
    can_datagram_set_data_buffer(s_data, TEST_DATA_SIZE);

    can_register_rx_handler(TEST_CAN_DT_MSG_ID, prv_can_datagram_rx_handler, NULL); 

    can_datagram_start_tx();
    
    Event e = { 0 };
    while(!can_datagram_tx_complete()) {
        while(event_process(&e) != STATUS_CODE_OK) {}
        can_datagram_process_event(&e);
    }    
}


/* void test_can_datagram_rx(void) {
    CanDatagramSettings settings = {
        .tx_cb = prv_tx_callback,
        .mode = CAN_DATAGRAM_MODE_RX,
    };
    can_datagram_init(&settings);
    can_datagram_rx(NULL, 0, true);
    Event e = { 0 };
    for (int i = 0; i < 6; i++) {
      while (event_process(&e) != STATUS_CODE_OK) {
      }
      can_datagram_process_event(&e);
      can_datagram_rx(s_rx_data + rx_data_counter, data_lengths[i], false);
      rx_data_counter += data_lengths[i];
    }

} */