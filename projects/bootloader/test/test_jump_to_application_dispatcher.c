#include "jump_to_application_dispatcher.h"
#include "bootloader_crc32.h"

#include "bootloader_datagram_defs.h"
#include "bootloader_events.h"
#include "crc32.h"
#include "dispatcher.h"
#include "interrupt.h"
#include "ms_test_helper_datagram.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CLIENT_SCRIPT_ID 0

static uint8_t s_status_msg = 0;

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

static uint8_t s_tx_data[TEST_DATA_LEN] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                            'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p' };

static uint8_t s_destination_nodes[DGRAM_MAX_DEST_NODES_SIZE];
static uint8_t s_rx_data[DGRAM_MAX_DATA_SIZE];

static uint16_t s_rx_data_len;

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  gpio_init();
  soft_timer_init();
  crc32_init();

  ms_test_helper_datagram_init(&s_test_can_storage, &s_test_can_settings, s_board_id,
                               &s_test_datagram_settings);
  dispatcher_init(s_board_id);
}

void teardown_test(void) {}

// tests to be developed