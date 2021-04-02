#include "fan_control_solar.h"

#include <stdbool.h>

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

// Declare static test variables
static GpioAddress s_overtemp_addr;
static GpioAddress s_fan_fail_addr;
static GpioAddress s_full_speed_addr;
static uint16_t s_full_speed_temp_threshold = 3;  // arbitrarily selected value for testing
static SolarMpptCount s_mppt_count;

static GpioState s_gpio_state;

void setup_test(void) {
  event_queue_init();
  gpio_init();
  interrupt_init();
  gpio_it_init();
  data_store_init();

  // Test Board A with 6 MPPTs uses virtual port A
  GpioAddress s_overtemp_addr = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress s_fan_fail_addr = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress s_full_speed_addr = { .port = GPIO_PORT_A, .pin = 2 };

  SolarMpptCount s_mppt_count = SOLAR_BOARD_6_MPPTS;
}

// Used to reinitialize static testing variable GpioState s_gpio_state
void teardown_test(void) {
  s_gpio_state = GPIO_STATE_LOW;
}

// Test that "overtemperature" & "fan fail" faults are raised properly
void test_fan_fail_fault_handling(void) {
  FanControlSolarSettings settings = {
    .overtemp_addr = s_overtemp_addr,
    .fan_fail_addr = s_fan_fail_addr,
    .full_speed_addr = s_full_speed_addr,
    .full_speed_temp_threshold_dC = s_full_speed_temp_threshold,
    .mppt_count = s_mppt_count,
  };

  TEST_ASSERT_OK(fan_control_init(&settings));

  // Test that there are no faults initially
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Test that fan fail fault is raised correctly when fan fail pin is interrupted
  gpio_it_trigger_interrupt(&s_fan_fail_addr);

  Event e;
  MS_TEST_HELPER_ASSERT_EVENT(e, EE_SOLAR_FAULT_FAN_FAIL, 0);
}

// Test that overtemperature fault/pins & full speed pins are raised correctly
// Case 1: Temperature below threshold for all MPPTs
void test_fan_overtemp_fault_fullspeed_handling_case_belowtemp(void) {
  FanControlSolarSettings settings = {
    .overtemp_addr = s_overtemp_addr,
    .fan_fail_addr = s_fan_fail_addr,
    .full_speed_addr = s_full_speed_addr,
    .full_speed_temp_threshold_dC = s_full_speed_temp_threshold,
    .mppt_count = s_mppt_count,
  };

  // Ensure fan_control_init works
  TEST_ASSERT_OK(fan_control_init(&settings));

  for (Mppt i = 0; i < 6; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), BELOW_TEMPERATURE_THRESHOLD_DC);
  }
  fan_control_process_event(DATA_READY_EVENT);

  // Overtemperature fault should not be raised
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Full Speed pin should be high to disable full speed
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_gpio_state);
}

// Case 2: Temperature above threshold for one MPPT
void test_fan_overtemp_fault_fullspeed_handling_case_oneabovetemp(void) {
  FanControlSolarSettings settings = {
    .overtemp_addr = s_overtemp_addr,
    .fan_fail_addr = s_fan_fail_addr,
    .full_speed_addr = s_full_speed_addr,
    .full_speed_temp_threshold_dC = s_full_speed_temp_threshold,
    .mppt_count = s_mppt_count,
  };
  TEST_ASSERT_OK(fan_control_init(&settings));

  for (Mppt i = 0; i < 5; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), BELOW_TEMPERATURE_THRESHOLD_DC);
  }
  data_store_set(DATA_POINT_TEMPERATURE(6), ABOVE_TEMPERATURE_THRESHOLD_DC);
  fan_control_process_event(DATA_READY_EVENT);

  Event e;

  // Overtemperature fault should be raised
  // Should fail by default if event_process(&e) fails
  MS_TEST_HELPER_ASSERT_EVENT(e, EE_SOLAR_FAULT_FAN_OVERTEMPERATURE, 0);

  // Full Speed pin should be low to enable full speed
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_gpio_state);
}

// Case 3: Temperature above threshold for multiple MPPTs
void test_fan_overtemp_fault_fullspeed_handling_case_multipleabovetemp(void) {
  FanControlSolarSettings settings = {
    .overtemp_addr = s_overtemp_addr,
    .fan_fail_addr = s_fan_fail_addr,
    .full_speed_addr = s_full_speed_addr,
    .full_speed_temp_threshold_dC = s_full_speed_temp_threshold,
    .mppt_count = s_mppt_count,
  };
  TEST_ASSERT_OK(fan_control_init(&settings));

  for (Mppt i = 0; i < 6; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), ABOVE_TEMPERATURE_THRESHOLD_DC);
  }
  fan_control_process_event(DATA_READY_EVENT);

  Event e;

  // Overtemperature fault should be raised
  MS_TEST_HELPER_ASSERT_EVENT(e, EE_SOLAR_FAULT_FAN_OVERTEMPERATURE, 0);

  // Full Speed pin should be low to enable full speed
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_gpio_state);
}

// Case 4: Overtemperature status on any MPPT should set full speed pin low
void test_fan_overtemp_fault_fullspeed_handling_case_overtempstatus(void) {
  FanControlSolarSettings settings = {
    .overtemp_addr = s_overtemp_addr,
    .fan_fail_addr = s_fan_fail_addr,
    .full_speed_addr = s_full_speed_addr,
    .full_speed_temp_threshold_dC = s_full_speed_temp_threshold,
    .mppt_count = s_mppt_count,
  };
  TEST_ASSERT_OK(fan_control_init(&settings));

  // Set 6th mppt status to overtemperature
  data_store_set(DATA_POINT_MPPT_STATUS(5), SPV1020_OVT_MASK);

  fan_control_process_event(DATA_READY_EVENT);

  // Full Speed pin should be low to enable full speed
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_gpio_state);
}

// Test that full_speed set inactive (high) when MPPTs are no longer overtemp
void test_fan_overtemp_fault_fullspeed_handling_case_nolongerovertemp(void) {
  FanControlSolarSettings settings = {
    .overtemp_addr = s_overtemp_addr,
    .fan_fail_addr = s_fan_fail_addr,
    .full_speed_addr = s_full_speed_addr,
    .full_speed_temp_threshold_dC = s_full_speed_temp_threshold,
    .mppt_count = s_mppt_count,
  };
  TEST_ASSERT_OK(fan_control_init(&settings));

  // First cause full_speed pin to go active
  for (Mppt i = 0; i < 6; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), ABOVE_TEMPERATURE_THRESHOLD_DC);
  }
  fan_control_process_event(DATA_READY_EVENT);

  // Test that the full_speed pin is indeed active low
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_gpio_state);

  // Set temperatures below overtemp threshold
  for (Mppt i = 0; i < 6; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), BELOW_TEMPERATURE_THRESHOLD_DC);
  }
  fan_control_process_event(DATA_READY_EVENT);

  // Test that full_speed pin is inactive (high)
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_gpio_state);
}

// Test that initializing with invalid settings fails gracefully.
void test_invalid_settings(void) {
  // NULL init test
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, fan_control_init(NULL));

  // too many MPPTs test
  GpioAddress overtemp_addr = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress fan_fail_addr = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress full_speed_addr = { .port = GPIO_PORT_A, .pin = 2 };

  FanControlSolarSettings invalid_settings = {
    .overtemp_addr = overtemp_addr,
    .fan_fail_addr = fan_fail_addr,
    .full_speed_addr = full_speed_addr,
    .full_speed_temp_threshold_dC = 4,
    .mppt_count = MAX_SOLAR_BOARD_MPPTS + 1,
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, fan_control_init(&invalid_settings));
}

// Test that processing a non DATA_READY_EVENT returns false (fails gracefully)
void test_not_data_ready_event(void) {
  // NULL passed in
  TEST_ASSERT_FALSE(fan_control_process_event(NULL));

  // Non DATA_READY_EVENT passed in
  Event e;
  TEST_ASSERT_FALSE(fan_control_process_event(&e));
}
