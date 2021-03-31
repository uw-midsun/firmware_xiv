#include "fan_control_solar.h"

#include <stdbool.h>

#include "data_store.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fault_handler.h"
#include "fault_monitor.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "max6643_fan_controller.h"
#include "ms_test_helpers.h"
#include "solar_boards.h"
#include "solar_events.h"
#include "spv1020_mppt.h"
#include "test_helpers.h"
#include "unity.h"

// Any defines here

// Declare static test variables
static GpioAddress s_overtemp_addr;
static GpioAddress s_fan_fail_addr;
static GpioAddress s_full_speed_addr;
static uint16_t s_full_speed_temp_threshold;
static SolarMpptCount s_mppt_count;

static GpioState s_gpio_state;
static Event s_e;

static GpioAddress s_dummy;

static void TEST_MOCK(prv_fanfail_callback)(const GpioAddress *address, void *context) {
  LOG_WARN("fan_control detected fanfail, raising fault event");
  fault_handler_raise_fault(EE_SOLAR_FAULT_FAN_FAIL, 0);
}

void setup_test(void) {
  event_queue_init();
  gpio_init();
  gpio_it_init();
  data_store_init();

  // Test Board A with 6 MPPTs uses virtual port A
  GpioAddress s_overtemp_addr = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress s_fan_fail_addr = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress s_full_speed_addr = { .port = GPIO_PORT_A, .pin = 2 };

  // arbitrarily selected value for testing
  uint16_t s_full_speed_temp_threshold = 3;

  SolarMpptCount s_mppt_count = SOLAR_BOARD_6_MPPTS;
}

// Used to reinitialize static testing variables Event s_e & GpioState s_gpio_state
void teardown_test(void) {
  static GpioState s_gpio_state;
  static Event s_e;
}

// Test that "overtemperature" & "fan fail" faults are raised properly
void test_fan_fail_fault_handling(void) {
  FanControlSettingsSolar settings = {
    .overtemp_addr = s_overtemp_addr,
    .fan_fail_addr = s_fan_fail_addr,
    .full_speed_addr = s_full_speed_addr,
    .full_speed_temp_threshold = s_full_speed_temp_threshold,
    .mppt_count = s_mppt_count,
  };

  TEST_ASSERT_OK(fan_control_init(&settings));

  // Test that there are no faults initially
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Test that fan fail fault is raised correctly
  // The MAX6643 will call the prv_callback function when fan fail is detected
  // Parameters have no effect on function
  prv_fanfail_callback(&s_dummy);
  event_process(&s_e);
  MS_TEST_HELPER_ASSERT_EVENT(s_e, EE_SOLAR_FAULT_FAN_FAIL, 0);
}

// Test that overtemperature fault/pins & full speed pins are raised correctly
// Case 1: Temperature below threshold for all MPPTs
void test_fan_overtemp_fault_fullspeed_handling_case_belowtemp(void) {
  FanControlSettingsSolar settings = {
    .overtemp_addr = s_overtemp_addr,
    .fan_fail_addr = s_fan_fail_addr,
    .full_speed_addr = s_full_speed_addr,
    .full_speed_temp_threshold = s_full_speed_temp_threshold,
    .mppt_count = s_mppt_count,
  };

  // Ensure fan_control_init works
  TEST_ASSERT_OK(fan_control_init(&settings));

  for (Mppt i = 1; i <= 6; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), 2);
  }
  data_store_done();

  // Overtemperature fault should not be raised
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Full Speed pin should be high to disable full speed
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_gpio_state);
}

// Case 2: Temperature above threshold for one MPPT
void test_fan_overtemp_fault_fullspeed_handling_case_oneabovetemp(void) {
  FanControlSettingsSolar settings = {
    .overtemp_addr = s_overtemp_addr,
    .fan_fail_addr = s_fan_fail_addr,
    .full_speed_addr = s_full_speed_addr,
    .full_speed_temp_threshold = s_full_speed_temp_threshold,
    .mppt_count = s_mppt_count,
  };
  TEST_ASSERT_OK(fan_control_init(&settings));

  for (Mppt i = 1; i <= 5; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), 2);
  }
  data_store_set(DATA_POINT_TEMPERATURE(6), 4);
  data_store_done();

  // Overtemperature fault should be raised
  TEST_ASSERT_OK(event_process(&s_e));  // Make sure an event exists
  // Should fail by default if event_process(&e) fails
  MS_TEST_HELPER_ASSERT_EVENT(s_e, EE_SOLAR_FAULT_FAN_OVERTEMPERATURE, 0);

  // Full Speed pin should be low to enable full speed
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_gpio_state);
}

// Case 3: Temperature above threshold for multiple MPPTs
void test_fan_overtemp_fault_fullspeed_handling_case_multipleabovetemp(void) {
  FanControlSettingsSolar settings = {
    .overtemp_addr = s_overtemp_addr,
    .fan_fail_addr = s_fan_fail_addr,
    .full_speed_addr = s_full_speed_addr,
    .full_speed_temp_threshold = s_full_speed_temp_threshold,
    .mppt_count = s_mppt_count,
  };
  TEST_ASSERT_OK(fan_control_init(&settings));

  for (Mppt i = 1; i <= 6; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), 5);
  }
  data_store_done();

  // Overtemperature fault should be raised
  TEST_ASSERT_OK(event_process(&s_e));  // Make sure an event exists
  MS_TEST_HELPER_ASSERT_EVENT(s_e, EE_SOLAR_FAULT_FAN_OVERTEMPERATURE, 0);

  // Full Speed pin should be low to enable full speed
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_gpio_state);
}

// Case 4: Overtemperature status on any MPPT should set full speed pin low
void test_fan_overtemp_fault_fullspeed_handling_case_overtempstatus(void) {
  FanControlSettingsSolar settings = {
    .overtemp_addr = s_overtemp_addr,
    .fan_fail_addr = s_fan_fail_addr,
    .full_speed_addr = s_full_speed_addr,
    .full_speed_temp_threshold = s_full_speed_temp_threshold,
    .mppt_count = s_mppt_count,
  };
  TEST_ASSERT_OK(fan_control_init(&settings));

  // Set mppt(6)'s status to overtemperature
  data_store_set(DATA_POINT_MPPT_STATUS(6), 2);  // "0000010"

  // Full Speed pin should be low to enable full speed
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_gpio_state);
}

// Test that full_speed set inactive (high) when MPPTs are no longer overtemp
void test_fan_overtemp_fault_fullspeed_handling_case_nolongerovertemp(void) {
  FanControlSettingsSolar settings = {
    .overtemp_addr = s_overtemp_addr,
    .fan_fail_addr = s_fan_fail_addr,
    .full_speed_addr = s_full_speed_addr,
    .full_speed_temp_threshold = s_full_speed_temp_threshold,
    .mppt_count = s_mppt_count,
  };
  TEST_ASSERT_OK(fan_control_init(&settings));

  // First cause full_speed pin to go active
  for (Mppt i = 1; i <= 6; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), 10);
  }
  data_store_done();

  // Test that the full_speed pin is indeed active low
  gpio_get_state(&s_full_speed_addr, &s_gpio_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_gpio_state);

  // Set temperatures below overtemp threshold
  for (Mppt i = 1; i <= 6; i++) {
    data_store_set(DATA_POINT_TEMPERATURE(i), 1);
  }
  data_store_done();

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

  FanControlSettingsSolar invalid_settings = {
    .overtemp_addr = overtemp_addr,
    .fan_fail_addr = fan_fail_addr,
    .full_speed_addr = full_speed_addr,
    .full_speed_temp_threshold = 4,
    .mppt_count = MAX_SOLAR_BOARD_MPPTS + 1,
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, fan_control_init(&invalid_settings));
}
