// Note: we rely upon the fact that blink_event_generator raises an event immediately to detect
// state transitions.

#include "can.h"
#include "can_msg_defs.h"
#include "delay.h"
#include "event_queue.h"
#include "interrupt.h"
#include "lights_signal_fsm.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_BLINK_INTERVAL_US 30000
#define TEST_BUFFER_US 1000  // time after the blink should have happened that we check
#define TEST_BLINKS_BETWEEN_SYNCS 2

// Sending sync events requires that we're rear power distribution
#define TEST_CAN_DEVICE_ID SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR

#define TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(event_id, event_data) \
  do {                                                               \
    const Event e = { 0 };                                           \
    TEST_ASSERT_OK(event_process(&e));                               \
    TEST_ASSERT_EQUAL((event_id), e.id);                             \
    TEST_ASSERT_EQUAL((event_data), e.data);                         \
  } while (0)

#define TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED() \
  do {                                              \
    const Event e = { 0 };                          \
    StatusCode s = event_process(&e);               \
    TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, s);        \
  } while (0)

typedef enum {
  TEST_SIGNAL_INPUT_EVENT_LEFT = 0,
  TEST_SIGNAL_INPUT_EVENT_RIGHT,
  TEST_SIGNAL_INPUT_EVENT_HAZARD,
  TEST_SIGNAL_OUTPUT_EVENT_LEFT,
  TEST_SIGNAL_OUTPUT_EVENT_RIGHT,
  TEST_SIGNAL_OUTPUT_EVENT_HAZARD,
  TEST_SYNC_EVENT,
  NUM_TEST_SIGNAL_EVENTS,
} TestSignalEvent;

typedef enum {
  TEST_CAN_EVENT_RX = NUM_TEST_SIGNAL_EVENTS + 1,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

static SignalFsmStorage s_storage;
static const SignalFsmSettings s_settings = {
  .signal_left_input_event = TEST_SIGNAL_INPUT_EVENT_LEFT,
  .signal_right_input_event = TEST_SIGNAL_INPUT_EVENT_RIGHT,
  .signal_hazard_input_event = TEST_SIGNAL_INPUT_EVENT_HAZARD,
  .signal_left_output_event = TEST_SIGNAL_OUTPUT_EVENT_LEFT,
  .signal_right_output_event = TEST_SIGNAL_OUTPUT_EVENT_RIGHT,
  .signal_hazard_output_event = TEST_SIGNAL_OUTPUT_EVENT_HAZARD,
  .sync_event = TEST_SYNC_EVENT,
  .sync_behaviour = LIGHTS_SYNC_BEHAVIOUR_NO_SYNC,  // to change in a test, copy it and overwrite
  .num_blinks_between_syncs = TEST_BLINKS_BETWEEN_SYNCS,
  .blink_interval_us = TEST_BLINK_INTERVAL_US,
};

static const Event s_left_on_event = { .id = TEST_SIGNAL_INPUT_EVENT_LEFT, .data = 1 };
static const Event s_left_off_event = { .id = TEST_SIGNAL_INPUT_EVENT_LEFT, .data = 0 };
static const Event s_right_on_event = { .id = TEST_SIGNAL_INPUT_EVENT_RIGHT, .data = 1 };
static const Event s_right_off_event = { .id = TEST_SIGNAL_INPUT_EVENT_RIGHT, .data = 0 };
static const Event s_hazard_on_event = { .id = TEST_SIGNAL_INPUT_EVENT_HAZARD, .data = 1 };
static const Event s_hazard_off_event = { .id = TEST_SIGNAL_INPUT_EVENT_HAZARD, .data = 0 };
static const Event s_sync_event = { .id = TEST_SYNC_EVENT, .data = 0 };

static CanStorage s_can_storage;

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = TEST_CAN_DEVICE_ID,
    .loopback = false,  // we don't care about receiving the sync msgs we've sent
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_CAN_EVENT_RX,
    .tx_event = TEST_CAN_EVENT_TX,
    .fault_event = TEST_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  can_init(&s_can_storage, &can_settings);
}
void teardown_test(void) {}

// Test that we can successfully initialize.
void test_lights_signal_fsm_init_ok(void) {
  TEST_ASSERT_OK(lights_signal_fsm_init(&s_storage, &s_settings));
}

// Test that we actually blink.
void test_lights_signal_fsm_blinking(void) {
  TEST_ASSERT_OK(lights_signal_fsm_init(&s_storage, &s_settings));

  // transition to hazard
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_hazard_on_event));

  // make sure we're blinking long enough to see there's no sync event (since behaviour is none)
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 1);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_INTERVAL_US);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_INTERVAL_US);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 1);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_sync_event));  // should do nothing
  delay_us(TEST_BLINK_INTERVAL_US);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_INTERVAL_US);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 1);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // transition to none and make sure we're no longer blinking (but that we turn off the light)
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_hazard_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 0);
  delay_us(2 * TEST_BLINK_INTERVAL_US);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
}

// Test transitioning: none -> left -> none -> right -> none.
// Events given: left on, left off, right on, right off.
void test_lights_signal_fsm_none_left_none_right_none(void) {
  TEST_ASSERT_OK(lights_signal_fsm_init(&s_storage, &s_settings));

  // begin: none
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // left on: transition to left
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 1);

  // left off: transition to none
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 0);

  // right on: transition to right
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_right_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_RIGHT, 1);

  // right off: transition to none
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_right_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_RIGHT, 0);
}

// Test transitioning: none -> hazard -> hazard-left -> left -> none
// Events given: hazard on, left on, hazard off, left off
void test_lights_signal_fsm_none_hazard_hazardleft_left_none(void) {
  TEST_ASSERT_OK(lights_signal_fsm_init(&s_storage, &s_settings));

  // begin: none
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // hazard on: transition to hazard
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_hazard_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 1);

  // left on: transition to hazard-left (no behaviour change)
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // hazard off: transition to left
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_hazard_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 1);

  // left off: transition to none
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 0);
}

// Test transitioning: none -> left -> hazard-left -> hazard -> none
// Events given: left on, hazard on, left off, hazard off
void test_lights_signal_fsm_none_left_hazardleft_hazard_none(void) {
  TEST_ASSERT_OK(lights_signal_fsm_init(&s_storage, &s_settings));

  // begin: none
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // left on: transition to left
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 1);

  // hazard on: transition to hazard-left
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_hazard_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 1);

  // left off: transition to hazard (no behaviour change)
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // hazard off: transition to none
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_hazard_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 0);
}

// Test that we restart when we receive a sync event and behaviour is set to receive.
void test_lights_signal_fsm_restarts_receiving_sync_event(void) {
  SignalFsmSettings sync_receive_settings = s_settings;
  sync_receive_settings.sync_behaviour = LIGHTS_SYNC_BEHAVIOUR_RECEIVE_SYNC_MSGS;
  TEST_ASSERT_OK(lights_signal_fsm_init(&s_storage, &sync_receive_settings));

  // begin: none
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // receive a sync event here: nothing
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_sync_event));
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // left on: transition to left
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 1);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // sending a sync event causes it to restart
  delay_us(TEST_BLINK_INTERVAL_US / 2);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_sync_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 1);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_INTERVAL_US / 2);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_INTERVAL_US / 2 + TEST_BUFFER_US);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // hazard on: transition to hazard-left, test sync event restarting
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_hazard_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 1);
  delay_us(TEST_BLINK_INTERVAL_US + TEST_BUFFER_US);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_sync_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 1);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // left off: transition to hazard, test sync event restarting
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_sync_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 1);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_INTERVAL_US + TEST_BUFFER_US);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // right on: transition to hazard-right, test sync event restarting
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_right_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_sync_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 1);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // hazard off: transition to right, test sync event restarting
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_hazard_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_HAZARD, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_RIGHT, 1);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_INTERVAL_US + TEST_BUFFER_US);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_RIGHT, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_sync_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_RIGHT, 1);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // right off: transition to none, sync event does nothing
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_right_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_RIGHT, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_sync_event));
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
}

// Test that we send sync CAN msgs when in SYNC_BEHAVIOUR_SEND_SYNC_MSGS.
void test_lights_signal_fsm_sends_sync_msgs(void) {
  SignalFsmSettings send_settings = s_settings;
  send_settings.sync_behaviour = LIGHTS_SYNC_BEHAVIOUR_SEND_SYNC_MSGS;
  send_settings.num_blinks_between_syncs = TEST_BLINKS_BETWEEN_SYNCS;
  TEST_ASSERT_OK(lights_signal_fsm_init(&s_storage, &send_settings));

  // nothing happens at the beginning (CAN messages use TX events)
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_INTERVAL_US + TEST_BUFFER_US);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // left on: transition to left, wait for the sync event
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_on_event));
  for (uint8_t i = 0; i < TEST_BLINKS_BETWEEN_SYNCS - 1; i++) {
    TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 1);
    TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
    delay_us(TEST_BLINK_INTERVAL_US + (i == 0 ? TEST_BUFFER_US : 0));  // extra delay first time
    TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 0);
    TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
    delay_us(TEST_BLINK_INTERVAL_US);
  }
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 1);
  MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // wait another cycle, make sure we see another sync event
  delay_us(TEST_BLINK_INTERVAL_US);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_INTERVAL_US);
  for (uint8_t i = 0; i < TEST_BLINKS_BETWEEN_SYNCS - 1; i++) {
    TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 1);
    TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
    delay_us(TEST_BLINK_INTERVAL_US);
    TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 0);
    TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
    delay_us(TEST_BLINK_INTERVAL_US);
  }
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 1);
  MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_INTERVAL_US);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(TEST_SIGNAL_OUTPUT_EVENT_LEFT, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // left off: transition back to none, make sure there's no signal event
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  delay_us(3 * TEST_BLINK_INTERVAL_US);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
}
