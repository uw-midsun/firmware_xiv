#include <string.h>
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "power_main_sequence.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

static PowerMainSequenceFsmStorage s_sequence_storage;
static CanStorage s_can_storage;
static EERelayState s_battery_relay_state;
static EEDriveOutput s_drive_output;

static StatusCode prv_rx_relay_state_callback(const CanMessage *msg, void *context,
                                              CanAckStatus *ack_reply) {
  uint16_t relay_state;
  uint16_t relay_id;
  CAN_UNPACK_SET_RELAY_STATES(msg, &relay_id, &relay_state);
  TEST_ASSERT_EQUAL(relay_id, EE_RELAY_ID_BATTERY);
  s_battery_relay_state = relay_state;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  const CanSettings s_can_settings = { .device_id = SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,
                                       .loopback = true,
                                       .bitrate = CAN_HW_BITRATE_500KBPS,
                                       .rx = { .port = GPIO_PORT_A, .pin = 11 },
                                       .tx = { .port = GPIO_PORT_A, .pin = 12 },
                                       .rx_event = CENTRE_CONSOLE_EVENT_CAN_RX,
                                       .tx_event = CENTRE_CONSOLE_EVENT_CAN_TX,
                                       .fault_event = CENTRE_CONSOLE_EVENT_CAN_FAULT };
  TEST_ASSERT_OK(can_init(&s_can_storage, &s_can_settings));
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_SET_RELAY_STATES,
                                         prv_rx_relay_state_callback, NULL));
}

void teardown_test(void) {}

void test_happy_path(void) {}
