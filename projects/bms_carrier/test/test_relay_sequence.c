#include <string.h>

#include "bms_events.h"
#include "can.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "delay.h"
#include "exported_enums.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "mcp23008_gpio_expander.h"
#include "ms_test_helpers.h"
#include "relay_sequence.h"

// used for manually triggering interrupts
static GpioAddress s_io_expander_int = BMS_IO_EXPANDER_INT_PIN;

static CanStorage s_can_storage = { 0 };
static CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_BMS_CARRIER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = BMS_CAN_EVENT_RX,
  .tx_event = BMS_CAN_EVENT_TX,
  .fault_event = BMS_CAN_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

static Mcp23008GpioState s_23008_state_0 = NUM_MCP23008_GPIO_STATES;  // gnd
static Mcp23008GpioState s_23008_state_1 = NUM_MCP23008_GPIO_STATES;  // hv
StatusCode TEST_MOCK(mcp23008_gpio_get_state)(const Mcp23008GpioAddress *address,
                                              Mcp23008GpioState *input_state) {
  if (address->pin == 0) {
    *input_state = s_23008_state_0;
  }
  if (address->pin == 1) {
    *input_state = s_23008_state_1;
  }
  return STATUS_CODE_OK;
}

static GpioState s_hv_state = NUM_GPIO_STATES;
static GpioState s_gnd_state = NUM_GPIO_STATES;
StatusCode TEST_MOCK(gpio_set_state)(const GpioAddress *address, GpioState state) {
  if (address->pin == 0) {
    s_hv_state = state;
  } else if (address->pin == 1) {
    s_gnd_state = state;
  }
  return STATUS_CODE_OK;
}

static uint8_t s_fault_bps_bitmask = 0;
static uint8_t s_fault_bps_calls = 0;
void TEST_MOCK(fault_bps)(uint8_t bitmask, bool clear) {
  s_fault_bps_bitmask = bitmask;
  s_fault_bps_calls++;
}

static RelayStorage s_storage = { 0 };

void setup_test(void) {
  s_23008_state_0 = NUM_MCP23008_GPIO_STATES;
  s_23008_state_1 = NUM_MCP23008_GPIO_STATES;
  s_hv_state = NUM_GPIO_STATES;
  s_gnd_state = NUM_GPIO_STATES;
  s_fault_bps_bitmask = 0;
  s_fault_bps_calls = 0;
  memset(&s_storage, 0, sizeof(s_storage));
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  can_init(&s_can_storage, &s_can_settings);
}

void teardown_test(void) {}

void test_io_int(void) {
  TEST_ASSERT_OK(relay_sequence_init(&s_storage));
  // test storage is set correctly
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_LOW, s_storage.gnd_enabled);
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_LOW, s_storage.hv_enabled);
  s_23008_state_0 = MCP23008_GPIO_STATE_HIGH;
  s_23008_state_1 = MCP23008_GPIO_STATE_HIGH;
  gpio_it_trigger_interrupt(&s_io_expander_int);
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_HIGH, s_storage.gnd_enabled);
  TEST_ASSERT_EQUAL(MCP23008_GPIO_STATE_HIGH, s_storage.hv_enabled);
}

void test_open_good(void) {
  TEST_ASSERT_OK(relay_sequence_init(&s_storage));
  // start test in "closed" state
  s_storage.hv_enabled = true;
  s_storage.gnd_enabled = true;
  s_23008_state_0 = MCP23008_GPIO_STATE_HIGH;
  s_23008_state_1 = MCP23008_GPIO_STATE_HIGH;
  s_hv_state = GPIO_STATE_HIGH;
  s_gnd_state = GPIO_STATE_HIGH;
  TEST_ASSERT_OK(relay_open_sequence(&s_storage));
  // timers should be running
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.assertion_timer_id));
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.next_step_timer_id));
  // set the state to what would happen in hardware in the good case
  s_23008_state_0 = MCP23008_GPIO_STATE_LOW;
  s_23008_state_1 = MCP23008_GPIO_STATE_LOW;
  TEST_ASSERT_EQUAL(s_hv_state, GPIO_STATE_LOW);
  TEST_ASSERT_EQUAL(s_gnd_state, GPIO_STATE_LOW);
  gpio_it_trigger_interrupt(&s_io_expander_int);
  delay_ms(RELAY_SEQUENCE_ASSERTION_DELAY_MS + 5);
  // assertion should have triggered, next step should be waiting
  TEST_ASSERT_EQUAL(0, soft_timer_remaining_time(s_storage.assertion_timer_id));
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.next_step_timer_id));
  // no faults should have occured
  TEST_ASSERT_EQUAL(0, s_fault_bps_calls);
  TEST_ASSERT_EQUAL(0, s_fault_bps_bitmask);
  delay_ms(RELAY_SEQUENCE_NEXT_STEP_DELAY_MS - RELAY_SEQUENCE_ASSERTION_DELAY_MS + 5);
  // no timers should be running
  TEST_ASSERT_EQUAL(0, soft_timer_remaining_time(s_storage.assertion_timer_id));
  TEST_ASSERT_EQUAL(0, soft_timer_remaining_time(s_storage.next_step_timer_id));
  // state should be not enabled
  TEST_ASSERT_EQUAL(false, s_storage.hv_enabled);
  TEST_ASSERT_EQUAL(false, s_storage.gnd_enabled);
  // CAN message should be sent
  MS_TEST_HELPER_CAN_TX_RX(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
}

void test_open_bad(void) {
  TEST_ASSERT_OK(relay_sequence_init(&s_storage));
  // start test in "closed" state
  s_storage.hv_enabled = true;
  s_storage.gnd_enabled = true;
  s_23008_state_0 = MCP23008_GPIO_STATE_HIGH;
  s_23008_state_1 = MCP23008_GPIO_STATE_HIGH;
  TEST_ASSERT_OK(relay_open_sequence(&s_storage));
  // gpio should have been set
  TEST_ASSERT_EQUAL(s_hv_state, GPIO_STATE_LOW);
  TEST_ASSERT_EQUAL(s_gnd_state, GPIO_STATE_LOW);
  // timers should be running
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.assertion_timer_id));
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.next_step_timer_id));
  // set the state to what would happen in hardware in the bad case
  s_23008_state_0 = MCP23008_GPIO_STATE_HIGH;
  s_23008_state_1 = MCP23008_GPIO_STATE_LOW;
  gpio_it_trigger_interrupt(&s_io_expander_int);
  delay_ms(RELAY_SEQUENCE_ASSERTION_DELAY_MS + 5);
  // assertion should have triggered, next step should be waiting
  TEST_ASSERT_EQUAL(0, soft_timer_remaining_time(s_storage.assertion_timer_id));
  // next step timer should be cancelled
  TEST_ASSERT_EQUAL(0, soft_timer_remaining_time(s_storage.next_step_timer_id));
  // relay fault should have occured
  TEST_ASSERT_EQUAL(1, s_fault_bps_calls);
  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_RELAY, s_fault_bps_bitmask);
  // no can message should be sent
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void test_close_good(void) {
  TEST_ASSERT_OK(relay_sequence_init(&s_storage));
  // start test in "open" state
  s_storage.hv_enabled = false;
  s_storage.gnd_enabled = false;
  s_23008_state_0 = MCP23008_GPIO_STATE_LOW;
  s_23008_state_1 = MCP23008_GPIO_STATE_LOW;
  TEST_ASSERT_OK(relay_close_sequence(&s_storage));
  // timers should be running
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.assertion_timer_id));
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.next_step_timer_id));
  // set the state to what would happen in hardware in the good case
  s_23008_state_0 = MCP23008_GPIO_STATE_HIGH;
  s_23008_state_1 = MCP23008_GPIO_STATE_LOW;
  gpio_it_trigger_interrupt(&s_io_expander_int);
  delay_ms(RELAY_SEQUENCE_ASSERTION_DELAY_MS + 5);
  // assertion should have triggered, next step should be waiting
  TEST_ASSERT_EQUAL(0, soft_timer_remaining_time(s_storage.assertion_timer_id));
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.next_step_timer_id));
  // no faults should have occured
  TEST_ASSERT_EQUAL(0, s_fault_bps_calls);
  TEST_ASSERT_EQUAL(0, s_fault_bps_bitmask);
  delay_ms(RELAY_SEQUENCE_NEXT_STEP_DELAY_MS - RELAY_SEQUENCE_ASSERTION_DELAY_MS + 5);
  // state should be gnd on, hv off
  TEST_ASSERT_EQUAL(true, s_storage.gnd_enabled);
  TEST_ASSERT_EQUAL(false, s_storage.hv_enabled);
  TEST_ASSERT_EQUAL(0, s_fault_bps_calls);
  // timers for assertion and next step should be running
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.assertion_timer_id));
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.next_step_timer_id));
  // set the state to what would happen in hardware in the good case
  s_23008_state_0 = MCP23008_GPIO_STATE_HIGH;
  s_23008_state_1 = MCP23008_GPIO_STATE_HIGH;
  gpio_it_trigger_interrupt(&s_io_expander_int);
  delay_ms(RELAY_SEQUENCE_ASSERTION_DELAY_MS + 5);
  // assertion should have triggered, next step should be waiting
  TEST_ASSERT_EQUAL(0, soft_timer_remaining_time(s_storage.assertion_timer_id));
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.next_step_timer_id));
  // no faults should have occured
  TEST_ASSERT_EQUAL(0, s_fault_bps_calls);
  TEST_ASSERT_EQUAL(0, s_fault_bps_bitmask);
  delay_ms(RELAY_SEQUENCE_NEXT_STEP_DELAY_MS - RELAY_SEQUENCE_ASSERTION_DELAY_MS + 5);
  // state should be enabled
  TEST_ASSERT_EQUAL(true, s_storage.gnd_enabled);
  TEST_ASSERT_EQUAL(true, s_storage.hv_enabled);
  // no faults should have occured
  TEST_ASSERT_EQUAL(0, s_fault_bps_calls);
  // CAN message should be sent
  MS_TEST_HELPER_CAN_TX_RX(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
}

void test_close_gnd_bad(void) {
  TEST_ASSERT_OK(relay_sequence_init(&s_storage));
  // start test in "open" state
  s_storage.hv_enabled = false;
  s_storage.gnd_enabled = false;
  s_23008_state_0 = MCP23008_GPIO_STATE_LOW;
  s_23008_state_1 = MCP23008_GPIO_STATE_LOW;
  TEST_ASSERT_OK(relay_close_sequence(&s_storage));
  // timers should be running
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.assertion_timer_id));
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.next_step_timer_id));
  // set the state to what would happen in hardware in the bad case
  s_23008_state_0 = MCP23008_GPIO_STATE_LOW;
  s_23008_state_1 = MCP23008_GPIO_STATE_LOW;
  gpio_it_trigger_interrupt(&s_io_expander_int);
  delay_ms(RELAY_SEQUENCE_ASSERTION_DELAY_MS + 5);
  // assertion should have triggered, next step should be cancelled
  TEST_ASSERT_EQUAL(0, soft_timer_remaining_time(s_storage.assertion_timer_id));
  TEST_ASSERT_EQUAL(0, soft_timer_remaining_time(s_storage.next_step_timer_id));
  // fault should have occured
  TEST_ASSERT_EQUAL(1, s_fault_bps_calls);
  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_RELAY, s_fault_bps_bitmask);
}

void test_close_hv_bad(void) {
  TEST_ASSERT_OK(relay_sequence_init(&s_storage));
  // start test in "open" state
  s_storage.hv_enabled = false;
  s_storage.gnd_enabled = false;
  s_23008_state_0 = MCP23008_GPIO_STATE_LOW;
  s_23008_state_1 = MCP23008_GPIO_STATE_LOW;
  TEST_ASSERT_OK(relay_close_sequence(&s_storage));
  // set the state to what would happen in hardware in the good case (gnd)
  s_23008_state_0 = MCP23008_GPIO_STATE_HIGH;
  s_23008_state_1 = MCP23008_GPIO_STATE_LOW;
  gpio_it_trigger_interrupt(&s_io_expander_int);
  delay_ms(RELAY_SEQUENCE_NEXT_STEP_DELAY_MS + 5);
  // timers for assertion and next step should be running
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.assertion_timer_id));
  TEST_ASSERT_NOT_EQUAL(0, soft_timer_remaining_time(s_storage.next_step_timer_id));
  // set the state to what would happen in hardware in the bad case (hv)
  s_23008_state_0 = MCP23008_GPIO_STATE_HIGH;
  s_23008_state_1 = MCP23008_GPIO_STATE_LOW;
  gpio_it_trigger_interrupt(&s_io_expander_int);
  delay_ms(RELAY_SEQUENCE_ASSERTION_DELAY_MS + 5);
  // assertion should have triggered, next step should be cancelled
  TEST_ASSERT_EQUAL(0, soft_timer_remaining_time(s_storage.assertion_timer_id));
  TEST_ASSERT_EQUAL(0, soft_timer_remaining_time(s_storage.next_step_timer_id));
  // fault should have occured
  TEST_ASSERT_EQUAL(1, s_fault_bps_calls);
  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_RELAY, s_fault_bps_bitmask);
}

void test_centre_console_rx(void) {
  TEST_ASSERT_OK(relay_sequence_init(&s_storage));
  // test power on closes
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(NULL, EE_POWER_MAIN_SEQUENCE_CLOSE_BATTERY_RELAYS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  // should fault since we're not bothering to set hardware
  delay_ms(RELAY_SEQUENCE_ASSERTION_DELAY_MS + 5);
  TEST_ASSERT_EQUAL(1, s_fault_bps_calls);
  // test power off opens
  CAN_TRANSMIT_POWER_OFF_SEQUENCE(NULL, EE_POWER_OFF_SEQUENCE_OPEN_BATTERY_RELAYS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  // should fault since we're not bothering to set hardware properly
  s_storage.gnd_expected_state = GPIO_STATE_HIGH;
  delay_ms(RELAY_SEQUENCE_ASSERTION_DELAY_MS + 5);
  TEST_ASSERT_EQUAL(2, s_fault_bps_calls);
}
