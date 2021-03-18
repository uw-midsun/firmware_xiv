#include "relay_fsm.h"

#include <stdbool.h>

#include "can.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "delay.h"
#include "drv120_relay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fault_handler.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "solar_config.h"
#include "solar_events.h"
#include "test_helpers.h"
#include "unity.h"

#define IRRELEVANT_EVENT NUM_SOLAR_RELAY_EVENTS

#define PROCESS_RELAY_FSM_EVENT(storage, e)    \
  ({                                           \
    MS_TEST_HELPER_AWAIT_EVENT(e);             \
    relay_fsm_process_event(&(storage), &(e)); \
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();   \
  })

static CanStorage s_can_storage;
static RelayFsmStorage s_storage;

static bool relay_err_rx_cb_called;
static StatusCode prv_err_rx_cb(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  relay_err_rx_cb_called = true;
  uint8_t solar_fault;
  uint8_t fault_data;
  CAN_UNPACK_SOLAR_FAULT_5_MPPTS(msg, &solar_fault, &fault_data);
  TEST_ASSERT_EQUAL(EE_SOLAR_FAULT_DRV120, solar_fault);
  TEST_ASSERT_EQUAL(0, fault_data);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_SOLAR_5_MPPTS,
                                  SOLAR_CAN_EVENT_TX, SOLAR_CAN_EVENT_RX, SOLAR_CAN_EVENT_FAULT);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_SOLAR_FAULT_5_MPPTS, prv_err_rx_cb, NULL);
  event_queue_init();
  gpio_it_init();
  TEST_ASSERT_OK(relay_fsm_init(&s_storage));
}
void teardown_test(void) {}

// Test that the relay starts closed.
void test_starts_closed(void) {
  bool closed;
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);
}

// Test that the relay can change in response to command functions.
void test_relay_transitions(void) {
  Event e = { 0 };
  bool closed;

  // reset to open
  TEST_ASSERT_OK(relay_fsm_open());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // open -> closed
  TEST_ASSERT_OK(relay_fsm_close());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // closed -> open
  TEST_ASSERT_OK(relay_fsm_open());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // open -> closed
  TEST_ASSERT_OK(relay_fsm_close());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);
}

// Test that trying to open or close the relay when it's already opened or closed does nothing.
void test_noop_relay_transitions(void) {
  Event e = { 0 };
  bool closed;

  // reset to open
  TEST_ASSERT_OK(relay_fsm_open());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // open still (no-op)
  TEST_ASSERT_OK(relay_fsm_open());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // open -> closed
  TEST_ASSERT_OK(relay_fsm_close());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // closed still (no-op)
  TEST_ASSERT_OK(relay_fsm_close());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);
}

// Test that the FSM doesn't respond to irrelevant events.
void test_irrelevant_events(void) {
  Event e = { 0 };
  bool closed;

  // reset to open
  TEST_ASSERT_OK(relay_fsm_open());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  e.id = IRRELEVANT_EVENT;
  relay_fsm_process_event(&s_storage, &e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);  // not changed
}

// Test that |relay_fsm_process_event| returns whether there was a state change.
void test_relay_fsm_process_event_return_value(void) {
  // force to closed at start - valid in case we don't initialize to closed in the future
  Event e = { 0 };
  TEST_ASSERT_OK(relay_fsm_close());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);

  e.id = IRRELEVANT_EVENT;
  TEST_ASSERT_FALSE(relay_fsm_process_event(&s_storage, &e));

  TEST_ASSERT_OK(relay_fsm_open());
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_TRUE(relay_fsm_process_event(&s_storage, &e));  // now open

  TEST_ASSERT_OK(relay_fsm_open());
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_FALSE(relay_fsm_process_event(&s_storage, &e));  // still open - no change

  TEST_ASSERT_OK(relay_fsm_close());
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_TRUE(relay_fsm_process_event(&s_storage, &e));  // now closed

  TEST_ASSERT_OK(relay_fsm_close());
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_FALSE(relay_fsm_process_event(&s_storage, &e));  // still closed - no change

  e.id = IRRELEVANT_EVENT;
  TEST_ASSERT_FALSE(relay_fsm_process_event(&s_storage, &e));
}

// Test that the module gracefully handles null and invalid inputs.
void test_invalid_input(void) {
  TEST_ASSERT_NOT_OK(relay_fsm_init(NULL));

  // force to closed so we can get a state changing event
  Event e = { 0 };
  TEST_ASSERT_OK(relay_fsm_close());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);

  TEST_ASSERT_OK(relay_fsm_open());
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_FALSE(relay_fsm_process_event(NULL, &e));
  TEST_ASSERT_FALSE(relay_fsm_process_event(&s_storage, NULL));
  TEST_ASSERT_FALSE(relay_fsm_process_event(NULL, NULL));
}

void test_relay_it_cb(void) {
  gpio_it_trigger_interrupt(config_get_drv120_status_pin());
  MS_TEST_HELPER_CAN_TX_RX(SOLAR_CAN_EVENT_TX, SOLAR_CAN_EVENT_RX);
  TEST_ASSERT_TRUE(relay_err_rx_cb_called);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
