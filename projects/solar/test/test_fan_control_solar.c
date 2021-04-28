// Module to be tested
#include "fan_control_solar.h"

#include "can_msg_defs.h"
#include "can_unpack.h"
#include "data_store.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fault_handler.h"
#include "fault_monitor.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "max6643_fan_controller.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "solar_boards.h"
#include "solar_events.h"
#include "spv1020_mppt.h"
#include "spv1020_mppt_defs.h"
#include "test_helpers.h"
#include "unity.h"

// Any defines here
#define BELOW_TEMPERATURE_THRESHOLD_DC 2
#define ABOVE_TEMPERATURE_THRESHOLD_DC 4
#define SPV1020_NOMINAL_VALUE 0b0000000

typedef enum {
  TEST_CAN_SOLAR_FAN_EVENT_TX = 0,
  TEST_CAN_SOLAR_FAN_EVENT_RX,
  TEST_CAN_SOLAR_FAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

// Declare static test variables
static const uint16_t s_full_speed_temp_threshold = 3;  // arbitrarily selected value for testing
static const SolarMpptCount s_mppt_count = SOLAR_BOARD_6_MPPTS;

// Test Board A with 6 MPPTs uses virtual port A
static GpioAddress s_overtemp_addr = { .port = GPIO_PORT_A, .pin = 0 };
static const GpioAddress s_fan_fail_addr = { .port = GPIO_PORT_A, .pin = 1 };
static const GpioAddress s_full_speed_addr = { .port = GPIO_PORT_A, .pin = 2 };

static FanControlSolarSettings s_fan_control_settings;

static Event s_data_ready = { .id = DATA_READY_EVENT, .data = 0 };
static GpioState s_gpio_state;
static CanStorage s_can_storage = { 0 };
static FaultHandlerSettings s_fault_handler_settings = {
  .relay_open_faults = {},
  .num_relay_open_faults = 0,
  .mppt_count = SOLAR_BOARD_6_MPPTS,
};

// To receive rx_fault_monitor info
static uint8_t s_ee_solar_fault;
static uint8_t s_fault_data;

static StatusCode prv_ee_solar_fault_rx_handler(const CanMessage *msg, void *context,
                                                CanAckStatus *ack_reply) {
  CAN_UNPACK_SOLAR_FAULT_6_MPPTS(msg, &s_ee_solar_fault, &s_fault_data);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  // Also runs event_queue_init(), gpio_init(), interrupt_init(), soft_timer_init()
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_SOLAR_6_MPPTS,
                                  TEST_CAN_SOLAR_FAN_EVENT_TX, TEST_CAN_SOLAR_FAN_EVENT_RX,
                                  TEST_CAN_SOLAR_FAN_EVENT_FAULT);
  gpio_it_init();
  data_store_init();
  fault_handler_init(&s_fault_handler_settings);

  // Register Handlers to receive and unpack CAN message into static variables
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_SOLAR_FAULT_6_MPPTS, prv_ee_solar_fault_rx_handler,
                          NULL);

  // Initialize fan control settings
  s_fan_control_settings.overtemp_addr = s_overtemp_addr;
  s_fan_control_settings.fan_fail_addr = s_fan_fail_addr;
  s_fan_control_settings.full_speed_addr = s_full_speed_addr;
  s_fan_control_settings.full_speed_temp_threshold_dC = s_full_speed_temp_threshold;
  s_fan_control_settings.mppt_count = s_mppt_count;

  // Used to reinitialize static testing variable GpioState s_gpio_state (default inactive high)
  s_gpio_state = GPIO_STATE_HIGH;
}

void teardown_test(void) {}

// Test that "fan fail" faults are raised properly
void test_fan_fail_fault_handling(void) {
  TEST_ASSERT_OK(fan_control_init(&s_fan_control_settings));

  // Test that there are no faults initially
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Test that fan fail fault is raised correctly when fan fail pin is interrupted
  gpio_it_trigger_interrupt(&s_fan_fail_addr);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_SOLAR_FAN_EVENT_TX, TEST_CAN_SOLAR_FAN_EVENT_RX);

  TEST_ASSERT_EQUAL_MESSAGE(EE_SOLAR_FAULT_FAN_FAIL, s_ee_solar_fault,
                            "Was expecting EE_SOLAR_FAULT_FAN_FAIL instead");
  TEST_ASSERT_EQUAL_INT8(0, s_fault_data);
}

// Test that "overtemperature" faults are raised properly
void test_overtemp_fault_handling(void) {
  TEST_ASSERT_OK(fan_control_init(&s_fan_control_settings));

  // Test that there are no faults initially
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Test that fan fail fault is raised correctly when fan fail pin is interrupted
  gpio_it_trigger_interrupt(&s_overtemp_addr);

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_SOLAR_FAN_EVENT_TX, TEST_CAN_SOLAR_FAN_EVENT_RX);

  TEST_ASSERT_EQUAL_MESSAGE(EE_SOLAR_FAULT_FAN_OVERTEMPERATURE, s_ee_solar_fault,
                            "Was expecting EE_SOLAR_FAULT_FAN_OVERTEMPERATURE instead");
  TEST_ASSERT_EQUAL_INT8(0, s_fault_data);
}

// Test that full speed pins are raised correctly
// Case 1: Temperature below threshold for all MPPTs
void test_fan_overtemp_fault_fullspeed_handling_case_belowtemp(void) {
  // Ensure fan_control_init works
  TEST_ASSERT_OK(fan_control_init(&s_fan_control_settings));

  for (Mppt i = 0; i < 6; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), BELOW_TEMPERATURE_THRESHOLD_DC);
  }
  fan_control_process_event(&s_data_ready);

  // Full Speed pin should be high to disable full speed
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_gpio_state);
}

// Case 2: Temperature above threshold for one MPPT
void test_fan_overtemp_fault_fullspeed_handling_case_oneabovetemp(void) {
  TEST_ASSERT_OK(fan_control_init(&s_fan_control_settings));

  for (Mppt i = 0; i < 5; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), BELOW_TEMPERATURE_THRESHOLD_DC);
  }
  data_store_set(DATA_POINT_TEMPERATURE(5), ABOVE_TEMPERATURE_THRESHOLD_DC);
  fan_control_process_event(&s_data_ready);

  // Full Speed pin should be low to enable full speed
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_gpio_state);
}

// Case 3: Temperature above threshold for multiple MPPTs
void test_fan_overtemp_fault_fullspeed_handling_case_multipleabovetemp(void) {
  TEST_ASSERT_OK(fan_control_init(&s_fan_control_settings));

  for (Mppt i = 0; i < 6; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), ABOVE_TEMPERATURE_THRESHOLD_DC);
  }
  fan_control_process_event(&s_data_ready);

  // Full Speed pin should be low to enable full speed
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_gpio_state);
}

// Case 4: Overtemperature status on any MPPT should set full speed pin low
void test_fan_overtemp_fault_fullspeed_handling_case_overtempstatus(void) {
  TEST_ASSERT_OK(fan_control_init(&s_fan_control_settings));

  // Set 6th mppt status to overtemperature
  data_store_set(DATA_POINT_MPPT_STATUS(5), SPV1020_OVT_MASK);

  // No data set for mppt thermoresistors intentionally
  fan_control_process_event(&s_data_ready);

  // Full Speed pin should be low to enable full speed
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_gpio_state);
}

// Test that full_speed set inactive (high) when MPPTs no longer detecting overtemp
void test_fan_overtemp_fault_fullspeed_handling_case_nolongerovertemp(void) {
  TEST_ASSERT_OK(fan_control_init(&s_fan_control_settings));

  // First trigger full_speed pin to go active
  for (Mppt i = 0; i < 6; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), ABOVE_TEMPERATURE_THRESHOLD_DC);
  }
  fan_control_process_event(&s_data_ready);

  // Test that the full_speed pin is indeed active low
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_gpio_state);

  // Set temperatures below overtemp threshold
  for (Mppt i = 0; i < 6; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), BELOW_TEMPERATURE_THRESHOLD_DC);
  }
  fan_control_process_event(&s_data_ready);

  // Test that full_speed pin is inactive (high)
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_gpio_state);
}

// Test that full_speed set inactive (high)
// when MPPTs no longer detecting overtemp status
void test_fan_overtemp_fault_fullspeed_handling_case_nolongerovtempstatus(void) {
  TEST_ASSERT_OK(fan_control_init(&s_fan_control_settings));

  // First trigger full_speed pin to go active
  data_store_set(DATA_POINT_MPPT_STATUS(5), SPV1020_OVT_MASK);
  fan_control_process_event(&s_data_ready);

  // Test that the full_speed pin is indeed active low
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_gpio_state);

  // Set status to nominal
  data_store_set(DATA_POINT_MPPT_STATUS(5), SPV1020_NOMINAL_VALUE);
  fan_control_process_event(&s_data_ready);

  // Test that full_speed pin is inactive (high)
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_gpio_state);
}

// Test that initializing with invalid settings fails gracefully.
void test_invalid_settings(void) {
  // NULL init test
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, fan_control_init(NULL));

  // too many MPPTs test
  FanControlSolarSettings invalid_settings = {
    .overtemp_addr = s_overtemp_addr,
    .fan_fail_addr = s_fan_fail_addr,
    .full_speed_addr = s_full_speed_addr,
    .full_speed_temp_threshold_dC = s_full_speed_temp_threshold,
    .mppt_count = MAX_SOLAR_BOARD_MPPTS + 1,
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, fan_control_init(&invalid_settings));
}

// Test that processing a non DATA_READY_EVENT returns false (fails gracefully)
void test_not_data_ready_event(void) {
  // NULL passed in
  TEST_ASSERT_FALSE(fan_control_process_event(NULL));

  // Non DATA_READY_EVENT passed in
  Event e = { DATA_READY_EVENT + 1, 0 };
  TEST_ASSERT_FALSE(fan_control_process_event(&e));
}
