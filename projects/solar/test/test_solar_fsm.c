#include "solar_fsm.h"

#include <stdbool.h>

#include "can.h"
#include "can_transmit.h"
#include "delay.h"
#include "drv120_relay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "solar_events.h"
#include "test_helpers.h"
#include "unity.h"

#define UNRECOGNIZED_FAULT NUM_EE_SOLAR_FAULTS
#define TEST_FAULT(n) (NUM_EE_SOLAR_FAULTS + 1 + (n))

#define TEST_CAN_DELAY_US 10000

static void prv_set_fault_event(Event *e, EESolarFault fault) {
  e->id = SOLAR_FAULT_EVENT;
  e->data = FAULT_EVENT_DATA(fault, 0);
}

static const GpioAddress s_test_relay_pin = { .port = GPIO_PORT_A, .pin = 6 };

static CanStorage s_can_storage;

void setup_test(void) {
  event_queue_init();
  gpio_init();
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_SOLAR, SOLAR_CAN_EVENT_TX,
                                  SOLAR_CAN_EVENT_RX, SOLAR_CAN_EVENT_FAULT);
  drv120_relay_init(&s_test_relay_pin);
}
void teardown_test(void) {}

static void prv_process_all_events(SolarFsmStorage *storage) {
  Event e = { 0 };
  while (event_process(&e) == STATUS_CODE_OK) {
    solar_fsm_process_event(storage, &e);
    can_process_event(&e);  // for acks
  }
}

static void prv_send_set_battery_relay_can_message(SolarFsmStorage *storage, EERelayState state) {
  CanAckRequest ack_request = {
    .callback = NULL,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_SOLAR),
  };
  uint16_t relay_mask = 1 << EE_RELAY_ID_BATTERY;
  uint16_t relay_state = state << EE_RELAY_ID_BATTERY;
  CAN_TRANSMIT_SET_RELAY_STATES(&ack_request, relay_mask, relay_state);
  MS_TEST_HELPER_CAN_TX_RX(SOLAR_CAN_EVENT_TX, SOLAR_CAN_EVENT_RX);
  delay_us(TEST_CAN_DELAY_US);
  prv_process_all_events(storage);
}

// Test that the relay starts out closed, and a single fault event causes it to open.
void test_relay_opening_single_fault_event(void) {
  bool closed;
  SolarFsmSettings settings = {
    .relay_open_faults = { TEST_FAULT(0) },
    .num_relay_open_faults = 1,
  };
  SolarFsmStorage storage;
  TEST_ASSERT_OK(solar_fsm_init(&storage, &settings));

  // should start off with relay closed
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // raise an unrecognized event - nothing should happen
  Event e = { .id = UNRECOGNIZED_FAULT };
  solar_fsm_process_event(&storage, &e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // raising the fault event opens the relay
  prv_set_fault_event(&e, TEST_FAULT(0));
  solar_fsm_process_event(&storage, &e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // raising unrecognized/fault events don't cause it to reclose
  e.id = UNRECOGNIZED_FAULT;
  solar_fsm_process_event(&storage, &e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);
  prv_set_fault_event(&e, TEST_FAULT(0));
  solar_fsm_process_event(&storage, &e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);
}

// Test that the FSM opens/closes the relay correctly in response to command events.
void test_responding_command_events(void) {
  bool closed;
  SolarFsmSettings settings = {
    .relay_open_faults = { TEST_FAULT(0) },
    .num_relay_open_faults = 1,
  };
  SolarFsmStorage storage;
  TEST_ASSERT_OK(solar_fsm_init(&storage, &settings));

  // open
  prv_send_set_battery_relay_can_message(&storage, EE_RELAY_STATE_OPEN);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // open -> closed
  prv_send_set_battery_relay_can_message(&storage, EE_RELAY_STATE_CLOSE);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // closed -> open
  prv_send_set_battery_relay_can_message(&storage, EE_RELAY_STATE_OPEN);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // open still (should be no-op)
  prv_send_set_battery_relay_can_message(&storage, EE_RELAY_STATE_OPEN);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // open -> closed
  prv_send_set_battery_relay_can_message(&storage, EE_RELAY_STATE_CLOSE);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // closed still (should be no-op)
  prv_send_set_battery_relay_can_message(&storage, EE_RELAY_STATE_CLOSE);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);
}

// Test that the maximum number of fault events can all open the relay. We use command events to
// reclose the relay after each opening.
void test_relay_opening_multiple_fault_events(void) {
  bool closed;
  SolarFsmSettings settings = {
    .num_relay_open_faults = MAX_RELAY_OPEN_FAULTS,
  };
  for (uint8_t i = 0; i < MAX_RELAY_OPEN_FAULTS; i++) {
    settings.relay_open_faults[i] = TEST_FAULT(i);
  }
  SolarFsmStorage storage;
  TEST_ASSERT_OK(solar_fsm_init(&storage, &settings));

  // should start off with relay closed
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // try each fault event
  Event fault_event = { 0 };
  for (uint8_t i = 0; i < MAX_RELAY_OPEN_FAULTS; i++) {
    prv_set_fault_event(&fault_event, TEST_FAULT(i));
    solar_fsm_process_event(&storage, &fault_event);
    drv120_relay_get_is_closed(&closed);
    TEST_ASSERT_FALSE(closed);

    // reset using the CAN message
    prv_send_set_battery_relay_can_message(&storage, EE_RELAY_STATE_CLOSE);
    drv120_relay_get_is_closed(&closed);
    TEST_ASSERT_TRUE(closed);
  }

  // events that aren't mentioned don't open the relay
  prv_set_fault_event(&fault_event, TEST_FAULT(MAX_RELAY_OPEN_FAULTS));
  solar_fsm_process_event(&storage, &fault_event);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);
  fault_event.id = UNRECOGNIZED_FAULT;
  solar_fsm_process_event(&storage, &fault_event);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);
}

// Test that |solar_fsm_process_event| returns whether there was a state change.
// We only test return values for fault events since command events are an implementation detail.
void test_solar_fsm_process_event_return_value(void) {
  SolarFsmSettings settings = {
    .relay_open_faults = { TEST_FAULT(0), TEST_FAULT(1) },
    .num_relay_open_faults = 2,
  };
  SolarFsmStorage storage;
  TEST_ASSERT_OK(solar_fsm_init(&storage, &settings));

  // force to closed at start - valid in case we don't initialize to closed in the future
  prv_send_set_battery_relay_can_message(&storage, EE_RELAY_STATE_CLOSE);

  Event e = { .id = UNRECOGNIZED_FAULT };
  TEST_ASSERT_FALSE(solar_fsm_process_event(&storage, &e));

  prv_set_fault_event(&e, TEST_FAULT(0));
  TEST_ASSERT_TRUE(solar_fsm_process_event(&storage, &e));  // now open
  TEST_ASSERT_FALSE(solar_fsm_process_event(&storage, &e));

  prv_send_set_battery_relay_can_message(&storage, EE_RELAY_STATE_CLOSE);

  prv_set_fault_event(&e, TEST_FAULT(1));
  TEST_ASSERT_TRUE(solar_fsm_process_event(&storage, &e));  // now open

  e.id = UNRECOGNIZED_FAULT;
  TEST_ASSERT_FALSE(solar_fsm_process_event(&storage, &e));
}

// Test that the module gracefully handles null and invalid inputs.
void test_invalid_input(void) {
  SolarFsmSettings settings = {
    .relay_open_faults = { TEST_FAULT(0) },
    .num_relay_open_faults = 1,
  };
  SolarFsmStorage storage;
  TEST_ASSERT_NOT_OK(solar_fsm_init(NULL, &settings));
  TEST_ASSERT_NOT_OK(solar_fsm_init(&storage, NULL));
  TEST_ASSERT_NOT_OK(solar_fsm_init(NULL, NULL));

  settings.num_relay_open_faults = MAX_RELAY_OPEN_FAULTS + 1;
  TEST_ASSERT_NOT_OK(solar_fsm_init(&storage, &settings));

  // valid initialization for testing |solar_fsm_process_event|
  settings.num_relay_open_faults = 1;
  TEST_ASSERT_OK(solar_fsm_init(&storage, &settings));

  Event e;
  prv_set_fault_event(&e, TEST_FAULT(0));
  TEST_ASSERT_FALSE(solar_fsm_process_event(NULL, &e));
  TEST_ASSERT_FALSE(solar_fsm_process_event(&storage, NULL));
  TEST_ASSERT_FALSE(solar_fsm_process_event(NULL, NULL));
}
