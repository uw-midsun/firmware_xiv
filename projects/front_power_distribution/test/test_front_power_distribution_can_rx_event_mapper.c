#include "can.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "front_power_distribution_can_rx_event_mapper.h"
#include "front_power_distribution_events.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CAN_DEVICE_ID 0x1

#define PROCESS_DELAY_US 2000

#define TEST_RX_TO_EVENT(msg, can_msg_id, can_data_0, can_data_1, event_id, event_data) \
  ({                                                                                    \
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();                                            \
    (msg).msg_id = (can_msg_id);                                                        \
    (msg).data_u16[0] = (can_data_0);                                                   \
    (msg).data_u16[1] = (can_data_1);                                                   \
    TEST_ASSERT_OK(can_transmit(&(msg), NULL));                                         \
    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);                     \
    Event e = { 0, 0 };                                                                 \
    MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, (event_id), (event_data));                      \
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();                                            \
  })

#define TEST_RX_TO_ON_EVENT(msg, can_msg_id, can_data_0, on_id, event_id) \
  TEST_RX_TO_EVENT(msg, can_msg_id, can_data_0, on_id, event_id, 1)

#define TEST_RX_TO_OFF_EVENT(msg, can_msg_id, can_data_0, off_id, event_id) \
  TEST_RX_TO_EVENT(msg, can_msg_id, can_data_0, off_id, event_id, 0)

#define TEST_RX_TO_ON_OFF_EVENT(msg, can_msg_id, can_data_0, off_id, on_id, event_id) \
  ({                                                                                  \
    TEST_RX_TO_ON_EVENT(msg, can_msg_id, can_data_0, on_id, event_id);                \
    TEST_RX_TO_OFF_EVENT(msg, can_msg_id, can_data_0, off_id, event_id);              \
  })

#define TEST_RX_TO_ON_OFF_LIGHTS_EVENT(msg, light_type, event_id)                         \
  TEST_RX_TO_ON_OFF_EVENT(msg, SYSTEM_CAN_MESSAGE_LIGHTS, light_type, EE_LIGHT_STATE_OFF, \
                          EE_LIGHT_STATE_ON, event_id)

#define TEST_RX_TO_ON_OFF_POWER_EVENT(msg, output_type, event_id)           \
  TEST_RX_TO_ON_OFF_EVENT(msg, SYSTEM_CAN_MESSAGE_FRONT_POWER, output_type, \
                          EE_FRONT_POWER_DISTRIBUTION_OUTPUT_STATE_OFF,     \
                          EE_FRONT_POWER_DISTRIBUTION_OUTPUT_STATE_ON, event_id)

typedef enum {
  TEST_CAN_EVENT_RX = 10,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
} TestCanEvent;

static CanStorage s_can_storage;

void setup_test(void) {
  event_queue_init();
  gpio_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = TEST_CAN_DEVICE_ID,
    .loopback = true,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_CAN_EVENT_RX,
    .tx_event = TEST_CAN_EVENT_TX,
    .fault_event = TEST_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },  // probably fine for testing
    .rx = { GPIO_PORT_A, 11 },
  };
  can_init(&s_can_storage, &can_settings);
}
void teardown_test(void) {}

// Comprehensive happy-path test: receiving each type of CAN message gives each event ID
void test_can_rx_event_mapper_raises_each_event_type(void) {
  TEST_ASSERT_OK(front_power_distribution_can_rx_event_mapper_init());

  CanMessage msg = {
    .source_id = TEST_CAN_DEVICE_ID,
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 8,
  };

  TEST_RX_TO_ON_OFF_LIGHTS_EVENT(msg, EE_LIGHT_TYPE_DRL, FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRL);
  TEST_RX_TO_ON_OFF_LIGHTS_EVENT(msg, EE_LIGHT_TYPE_SIGNAL_LEFT,
                                 FRONT_POWER_DISTRIBUTION_SIGNAL_EVENT_LEFT);
  TEST_RX_TO_ON_OFF_LIGHTS_EVENT(msg, EE_LIGHT_TYPE_SIGNAL_RIGHT,
                                 FRONT_POWER_DISTRIBUTION_SIGNAL_EVENT_RIGHT);
  TEST_RX_TO_ON_OFF_LIGHTS_EVENT(msg, EE_LIGHT_TYPE_SIGNAL_HAZARD,
                                 FRONT_POWER_DISTRIBUTION_SIGNAL_EVENT_HAZARD);

  TEST_RX_TO_ON_OFF_POWER_EVENT(msg, EE_FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY,
                                FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY);
  TEST_RX_TO_ON_OFF_POWER_EVENT(msg, EE_FRONT_POWER_DISTRIBUTION_OUTPUT_STEERING,
                                FRONT_POWER_DISTRIBUTION_GPIO_EVENT_STEERING);
  TEST_RX_TO_ON_OFF_POWER_EVENT(msg, EE_FRONT_POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE,
                                FRONT_POWER_DISTRIBUTION_GPIO_EVENT_CENTRE_CONSOLE);
  TEST_RX_TO_ON_OFF_POWER_EVENT(msg, EE_FRONT_POWER_DISTRIBUTION_OUTPUT_PEDAL,
                                FRONT_POWER_DISTRIBUTION_GPIO_EVENT_PEDAL);
  TEST_RX_TO_ON_OFF_POWER_EVENT(msg, EE_FRONT_POWER_DISTRIBUTION_OUTPUT_DRL,
                                FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRL);
  // STROBE and DRIVER_FANS not implemented because what is their port?

  // horn is a one-off so there's no point having a macro to make it nice
  TEST_RX_TO_EVENT(msg, SYSTEM_CAN_MESSAGE_HORN, EE_HORN_STATE_OFF, 0,
                   FRONT_POWER_DISTRIBUTION_GPIO_EVENT_HORN, 0);
  TEST_RX_TO_EVENT(msg, SYSTEM_CAN_MESSAGE_HORN, EE_HORN_STATE_ON, 0,
                   FRONT_POWER_DISTRIBUTION_GPIO_EVENT_HORN, 1);
}

// Test that the module ignores messages with different IDs/type fields which presumably aren't
// meant for it.
void test_can_rx_event_mapper_ignores_other_messages(void) {
  TEST_ASSERT_OK(front_power_distribution_can_rx_event_mapper_init());

  // First, an message id not used by anything
  CanMessage msg = {
    .source_id = TEST_CAN_DEVICE_ID,
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 8,
    .msg_id = NUM_SYSTEM_CAN_MESSAGES,  // hopefully this doesn't trigger an assert in can...
    .data_u16 = { 0, 0, 0, 0 },
  };
  can_transmit(&msg, NULL);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Then, lights with an unused light type
  msg.msg_id = SYSTEM_CAN_MESSAGE_LIGHTS;
  msg.data_u16[0] = NUM_EE_LIGHT_TYPES;
  can_transmit(&msg, NULL);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Then, power with an unused output
  msg.msg_id = SYSTEM_CAN_MESSAGE_FRONT_POWER;
  msg.data_u16[0] = NUM_EE_FRONT_POWER_DISTRIBUTION_OUTPUTS;
  can_transmit(&msg, NULL);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
