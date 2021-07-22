#include "bootloader_can.h"
#include "can_datagram.h"
#include "dispatcher.h"
#include "ping.h"
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

  dispatcher_init();
}

void teardown_test(void) {}

void test_init(void) {
  // test ping init works
}

void test_ping_response(void) {
  // test a ping will respond to the ping command
}

void test_ping_no_response(void) {
  // test a ping will not respond to a different datagram id and/or node id
}
