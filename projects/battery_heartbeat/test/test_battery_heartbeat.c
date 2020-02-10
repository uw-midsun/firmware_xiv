#include "battery_heartbeat.h"
#include "can.h"
#include "can_unpack.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "test_helpers.h"

#define TEST_BATTERY_HEARTBEAT_PERIOD_MS 50

static CanStorage s_can;
static BatteryHeartbeatStorage s_battery_heartbeat;
static CanAckStatus s_ack_status;
static EEBatteryHeartbeatState s_heartbeat_state;

static StatusCode prv_battery_rx(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  *ack_reply = s_ack_status;

  uint8_t state = UINT8_MAX;
  CAN_UNPACK_BPS_HEARTBEAT(msg, &state);
  LOG_DEBUG("ACK request: Battery state %d\n", state);
  s_heartbeat_state = state;

  return STATUS_CODE_OK;
}

typedef enum {
  TEST_BATTERY_HEARTBEAT_EVENT_CAN_TX = 0,
  TEST_BATTERY_HEARTBEAT_EVENT_CAN_RX,
  TEST_BATTERY_HEARTBEAT_EVENT_CAN_FAULT,
} TestBatteryHeartbeatEvent;

void setup_test(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CanSettings settings = {
    .device_id = SYSTEM_CAN_DEVICE_BMS_CARRIER,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_BATTERY_HEARTBEAT_EVENT_CAN_RX,
    .tx_event = TEST_BATTERY_HEARTBEAT_EVENT_CAN_TX,
    .fault_event = TEST_BATTERY_HEARTBEAT_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_init(&s_can, &settings);

  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, prv_battery_rx, NULL));

  s_ack_status = CAN_ACK_STATUS_OK;
  s_heartbeat_state = UINT8_MAX;
  battery_heartbeat_init(&s_battery_heartbeat, TEST_BATTERY_HEARTBEAT_PERIOD_MS,
                         CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_BMS_CARRIER));
}

void teardown_test(void) {}

void test_battery_heartbeat_can(void) {
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_BATTERY_HEARTBEAT_EVENT_CAN_TX,
                                    TEST_BATTERY_HEARTBEAT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(CAN_ACK_STATUS_OK, s_ack_status);
  TEST_ASSERT_EQUAL(EE_BATTERY_HEARTBEAT_STATE_OK, s_heartbeat_state);

  LOG_DEBUG("Raising ACK Timeout Fault\n");

  // Testing ACK timeout
  s_ack_status = CAN_ACK_STATUS_TIMEOUT;

  // Looping through allowed number of faults
  for (size_t i = 0; i < BATTERY_HEARTBEAT_MAX_ACK_FAILS; i++) {
    MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_BATTERY_HEARTBEAT_EVENT_CAN_TX,
                                      TEST_BATTERY_HEARTBEAT_EVENT_CAN_RX);
  }

  // Will fault on next heartbeat
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_BATTERY_HEARTBEAT_EVENT_CAN_TX,
                                    TEST_BATTERY_HEARTBEAT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(CAN_ACK_STATUS_TIMEOUT, s_ack_status);
  TEST_ASSERT_EQUAL(EE_BATTERY_HEARTBEAT_STATE_FAULT_ACK_TIMEOUT, s_heartbeat_state);
}

void test_battery_heartbeat_basic(void) {
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_BATTERY_HEARTBEAT_EVENT_CAN_TX,
                                    TEST_BATTERY_HEARTBEAT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(CAN_ACK_STATUS_OK, s_ack_status);
  TEST_ASSERT_EQUAL(EE_BATTERY_HEARTBEAT_STATE_OK, s_heartbeat_state);

  // Raise fault
  LOG_DEBUG("Raising fault\n");
  battery_heartbeat_raise_fault(&s_battery_heartbeat, EE_BATTERY_HEARTBEAT_FAULT_SOURCE_KILLSWITCH);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_BATTERY_HEARTBEAT_EVENT_CAN_TX,
                                    TEST_BATTERY_HEARTBEAT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(EE_BATTERY_HEARTBEAT_STATE_FAULT_KILLSWITCH, s_heartbeat_state);

  // Clear fault
  LOG_DEBUG("Clearing fault\n");
  battery_heartbeat_clear_fault(&s_battery_heartbeat, EE_BATTERY_HEARTBEAT_FAULT_SOURCE_KILLSWITCH);
  delay_ms(TEST_BATTERY_HEARTBEAT_PERIOD_MS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_BATTERY_HEARTBEAT_EVENT_CAN_TX,
                                    TEST_BATTERY_HEARTBEAT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(EE_BATTERY_HEARTBEAT_STATE_OK, s_heartbeat_state);
}
