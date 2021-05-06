#include "brake_light_control.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "pedal_rx.h"
#include "test_helpers.h"
#include "unity.h"

typedef enum {
  TEST_CAN_BRAKE_LIGHT_EVENT_TX = 0,
  TEST_CAN_BRAKE_LIGHT_EVENT_RX,
  TEST_CAN_BRAKE_LIGHT_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

static CanStorage s_can_storage = { 0 };

// Stores unpacked data from RX handler messages
static uint8_t s_brake_lights_id;
static uint8_t s_brake_lights_state;

// Callback function for RX handler
static StatusCode prv_brake_lights_handler(const CanMessage *msg, void *context,
                                           CanAckStatus *ack_reply) {
  CAN_UNPACK_LIGHTS(msg, &s_brake_lights_id, &s_brake_lights_state);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  // Also runs event_queue_init(), gpio_init(), interrupt_init(), soft_timer_init()
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,
                                  TEST_CAN_BRAKE_LIGHT_EVENT_TX, TEST_CAN_BRAKE_LIGHT_EVENT_RX,
                                  TEST_CAN_BRAKE_LIGHT_EVENT_FAULT);

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS, prv_brake_lights_handler, NULL);

  s_brake_lights_id = 0;
  s_brake_lights_state = 0;
}

void teardown_test(void) {}

// Test that CAN message for brake light on is sent when pedal is pressed
// with PEDAL_MONITOR_STATE_CHANGE event raised
void test_can_brake_light_on_when_pedal_pressed(void) {
  // After event_raise and event_process PEDAL_MONITOR_STATE_CHANGE event
  // with pedal released state, brake_light_control_process_event is called
  Event e = { PEDAL_MONITOR_STATE_CHANGE, PEDAL_STATE_PRESSED };
  TEST_ASSERT_TRUE(brake_light_control_process_event(&e));

  // Check that a CAN message has been TX'ed with correct ID & state
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_BRAKE_LIGHT_EVENT_TX, TEST_CAN_BRAKE_LIGHT_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_LIGHT_TYPE_BRAKES, s_brake_lights_id);
  TEST_ASSERT_EQUAL(EE_LIGHT_STATE_ON, s_brake_lights_state);
}

// Test that CAN message for brake light off is sent when pedal is released
// with PEDAL_MONITOR_STATE_CHANGE event raised
void test_can_brake_light_off_when_pedal_released(void) {
  // After event_raise and event_process PEDAL_MONITOR_STATE_CHANGE event
  // with pedal released state, brake_light_control_process_event is called
  Event e = { PEDAL_MONITOR_STATE_CHANGE, PEDAL_STATE_RELEASED };
  TEST_ASSERT_TRUE(brake_light_control_process_event(&e));

  // Check that a CAN message has been TX'ed with correct ID & state
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_BRAKE_LIGHT_EVENT_TX, TEST_CAN_BRAKE_LIGHT_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_LIGHT_TYPE_BRAKES, s_brake_lights_id);
  TEST_ASSERT_EQUAL(EE_LIGHT_STATE_OFF, s_brake_lights_state);
}

// Test that processing a non DATA_READY_EVENT returns false (fails gracefully)
void test_not_data_ready_event(void) {
  // NULL passed in
  TEST_ASSERT_FALSE(brake_light_control_process_event(NULL));

  // Non DATA_READY_EVENT passed in
  Event e = { PEDAL_MONITOR_STATE_CHANGE + 1, 0 };
  TEST_ASSERT_FALSE(brake_light_control_process_event(&e));
}
