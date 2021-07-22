#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "can_datagram.h"
#include "dispatcher.h"
#include "unity.h"

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
  bootloader_can_init(&s_test_can_storage, &s_test_can_settings);
  can_datagram_init(&s_test_datagram_settings);
}

void teardown_test(void) {}

void test_init() {
  // test dispatcher can setup rx for datagrams without errors
}

void test_register_callback(void) {
  // test registering a callback does not cause any error
}

void test_callback(void) {
  // test a registered callback is called when datagram is recieved
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
