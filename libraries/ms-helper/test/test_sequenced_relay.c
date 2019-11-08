#include "can.h"
#include "can_transmit.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "sequenced_relay.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_SEQUENCED_RELAY_CAN_DEVICE_ID 0x1

#define TEST_SEQUENCED_RELAY_LEFT \
  { .port = GPIO_PORT_B, .pin = 3 }
#define TEST_SEQUENCED_RELAY_RIGHT \
  { .port = GPIO_PORT_B, .pin = 9 }
#define TEST_SEQUENCED_RELAY_DELAY_MS 10

static CanStorage s_can;
static SequencedRelayStorage s_sequenced_relay;

typedef enum {
  TEST_SEQUENCED_RELAY_EVENT_CAN_RX = 0,
  TEST_SEQUENCED_RELAY_EVENT_CAN_TX,
  TEST_SEQUENCED_RELAY_EVENT_CAN_FAULT,
} TestSequencedRelayEvent;

static StatusCode prv_ack_cb(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                             uint16_t num_remaining, void *context) {
  CanAckStatus *ack_status = context;
  *ack_status = status;

  LOG_DEBUG("CAN ACK status %d\n", status);

  return STATUS_CODE_OK;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = TEST_SEQUENCED_RELAY_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_SEQUENCED_RELAY_EVENT_CAN_RX,
    .tx_event = TEST_SEQUENCED_RELAY_EVENT_CAN_TX,
    .fault_event = TEST_SEQUENCED_RELAY_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&s_can, &can_settings));

  SequencedRelaySettings relay_settings = {
    .can_msg_id = SYSTEM_CAN_MESSAGE_MOTOR_RELAY,
    .left_relay = TEST_SEQUENCED_RELAY_LEFT,
    .right_relay = TEST_SEQUENCED_RELAY_RIGHT,
    .delay_ms = TEST_SEQUENCED_RELAY_DELAY_MS,
  };
  TEST_ASSERT_OK(sequenced_relay_init(&s_sequenced_relay, &relay_settings));
}

void teardown_test(void) {}

void test_sequenced_relay_can(void) {
  // Ask to close the relay
  volatile CanAckStatus status = NUM_CAN_ACK_STATUSES;
  CanAckRequest ack_request = {
    .callback = prv_ack_cb,
    .context = &status,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(TEST_SEQUENCED_RELAY_CAN_DEVICE_ID),
  };
  CAN_TRANSMIT_MOTOR_RELAY(&ack_request, EE_RELAY_STATE_CLOSE);

  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_SEQUENCED_RELAY_EVENT_CAN_TX,
                                    TEST_SEQUENCED_RELAY_EVENT_CAN_RX);
  TEST_ASSERT_OK(status);

  GpioAddress left_relay = TEST_SEQUENCED_RELAY_LEFT;
  GpioAddress right_relay = TEST_SEQUENCED_RELAY_RIGHT;

  // Make sure that both relays are now closed. We allow some delay before
  // checking for sequencing.
  delay_ms(TEST_SEQUENCED_RELAY_DELAY_MS);
  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(&left_relay, &state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);
  gpio_get_state(&right_relay, &state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);

  // Try opening the relays
  CAN_TRANSMIT_MOTOR_RELAY(&ack_request, EE_RELAY_STATE_OPEN);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_SEQUENCED_RELAY_EVENT_CAN_TX,
                                    TEST_SEQUENCED_RELAY_EVENT_CAN_RX);
  TEST_ASSERT_OK(status);

  // Opening the relays does not require sequencing, so don't delay
  state = NUM_GPIO_STATES;
  gpio_get_state(&left_relay, &state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, state);
  gpio_get_state(&right_relay, &state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, state);
}

void test_sequenced_relay_set(void) {
  // Close the relays
  TEST_ASSERT_OK(sequenced_relay_set_state(&s_sequenced_relay, EE_RELAY_STATE_CLOSE));

  GpioAddress left_relay = TEST_SEQUENCED_RELAY_LEFT;
  GpioAddress right_relay = TEST_SEQUENCED_RELAY_RIGHT;

  // Make sure that both relays are now closed. We allow some delay before
  // checking for sequencing.
  delay_ms(TEST_SEQUENCED_RELAY_DELAY_MS);
  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(&left_relay, &state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);
  gpio_get_state(&right_relay, &state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);

  // Opening the relays
  TEST_ASSERT_OK(sequenced_relay_set_state(&s_sequenced_relay, EE_RELAY_STATE_OPEN));

  // Opening the relays does not require sequencing, so don't delay
  state = NUM_GPIO_STATES;
  gpio_get_state(&left_relay, &state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, state);
  gpio_get_state(&right_relay, &state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, state);
}
