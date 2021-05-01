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
#include "pedal_monitor.h"
#include "pedal_rx.h"
#include "test_helpers.h"
#include "unity.h"

#define PEDAL_STATE_UPDATE_FREQUENCY_MS 100
#define PEDAL_STATE_THRESHOLD 50

typedef enum {
  TEST_CAN_BRAKE_LIGHT_EVENT_TX = 0,
  TEST_CAN_BRAKE_LIGHT_EVENT_RX,
  TEST_CAN_BRAKE_LIGHT_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

static CanStorage s_can_storage = { 0 };
static PedalMonitorStorage s_pedal_storage = { 0 };

// Mocked function from pedal_monitor.c to avoid dealing with soft_timer
static void TEST_MOCK(prv_update_state(SoftTimerId timer_id, void *context)) {
  PedalMonitorStorage *storage = context;
  PedalState current_state = storage->state;
  PedalValues pedal_values = pedal_rx_get_pedal_values(&storage->rx_storage);
  storage->state =
      (pedal_values.brake > PEDAL_STATE_THRESHOLD) ? PEDAL_STATE_PRESSED : PEDAL_STATE_RELEASED;
  brake_light_control_update(current_state, storage->state);
}

// Stores unpacked data from RX handler messages
static uint8_t s_brake_lights_id;
static uint8_t s_brake_lights_state;

// Callback function for RX handler
static StatusCode prv_brake_lights_handler(const CanMessage *msg, void *context,
                                           CanAckStatus *ack_reply) {
  CAN_UNPACK_LIGHTS(msg, s_brake_lights_id, s_brake_lights_state);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  // Also runs event_queue_init(), gpio_init(), interrupt_init(), soft_timer_init()
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,
                                  TEST_CAN_BRAKE_LIGHT_EVENT_TX,
                                  TEST_CAN_BRAKE_LIGHT_EVENT_RX,
                                  TEST_CAN_BRAKE_LIGHT_EVENT_FAULT);
  pedal_monitor_init();

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS, prv_brake_lights_handler, NULL);
}

void teardown_test(void) {}

// Test that CAN message for brake light on is sent when pedal is pressed
void test_CAN_brake_light_on_when_pedal_pressed(void){
  // Set pedal state to pressed by setting pedal values above threshold

  // Call the callback function (simulate soft_timer activation)

  // Check that a CAN message has been TX'ed with correct ID & state

}

// Test that CAN message for brake light off is sent when pedal is released
void test_CAN_brake_light_off_when_pedal_released(void){
  // Set pedal state to pressed by setting pedal values above threshold

  // Call the callback function (simulate soft_timer activation)

  // Check that a CAN message has been TX'ed with correct ID & state
  
}

// Test that no CAN messages are sent if pedal remains in one state
// (either pressed or released)
void test_no_CAN_msg_when_state_constant(void){

}
