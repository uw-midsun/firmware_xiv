#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "can.h"
#include "can_datagram.h"
#include "can_pack_impl.h"
#include "can_transmit.h"
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
#include "test_helpers.h"
#include "unity.h"

#define TEST_DATA_GRAM_ID 0
#define CLIENT_SCRIPT_CONTROLLER_BOARD_ID 0
#define NON_CLIENT_SCRIPT_CONTROLLER_BOARD_ID 2

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

void setup_test(void) {
  bootloader_can_init(&s_test_can_storage, &s_test_can_settings,
                      NON_CLIENT_SCRIPT_CONTROLLER_BOARD_ID);
  can_datagram_init(&s_test_datagram_settings);
}

void teardown_test(void) {}

void prv_dispatch_cb(uint8_t *data, uint16_t len, void *context) {
  bool *complete = context;
  *complete = true;
}

void test_dispatch(void) {
  // bool complete = false;
  // // test dispatch works with a callback
  // TEST_ASSERT_OK(dispatcher_register_callback(TEST_DATA_GRAM_ID, prv_dispatch_cb, &complete));

  // // needs to send a start can msg with CAN (not CAN datagram)
  // // then send contents with CAN

  // // Send mock start message
  // CanMessage msg = { 0 };
  // can_pack_impl_empty(&msg, TEST_CAN_DEVICE_ID, TEST_CAN_START_MSG_ID);
  // can_transmit(&msg, NULL);
  // MS_TEST_HELPER_CAN_TX_RX(CAN_DATAGRAM_EVENT_TX, CAN_DATAGRAM_EVENT_RX);
  // TEST_ASSERT_EQUAL(true, s_start_message_set);

  // Event e = { 0 };
  // bool send_tx = true;
  // while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {  // Loop until rx complete
  //   // Send txes one at a time -> can datagram will send 4 at a time
  //   // but client in real scenario does not have to process tx's as well as rx's
  //   if (send_tx) {
  //     prv_mock_dgram_tx();
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
}

void test_datagram_completeness(void) {
  // check the data integrity when a datagram is recieved
}

void test_noexistant_callback(void) {
  // send a datagram id with no associated callback
}

//
void test_register_invalid_id(void) {
  TEST_ASSERT_NOT_OK(dispatcher_register_callback(NUM_BOOTLOADER_DATAGRAMS, NULL, NULL));
}
