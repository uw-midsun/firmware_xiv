#include "sense_mppt.h"

#include <stdbool.h>
#include <stdint.h>

#include "data_store.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "log.h"
#include "mppt.h"
#include "ms_test_helpers.h"
#include "sense.h"
#include "solar_boards.h"
#include "spv1020_mppt_defs.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_SPI_PORT SPI_PORT_2

#define INVALID_MPPT 99

#define TEST_CURRENT 0x1337
#define TEST_VOLTAGE 0xDEAD
#define TEST_PWM 0xBEEF
#define TEST_STATUS 1  // CR bit 1, other bits 0: OK

#define MAX_FAULTS (2 * NUM_EE_SOLAR_FAULTS * MAX_SOLAR_BOARD_MPPTS)

static SenseCallback s_sense_callbacks[MAX_SOLAR_BOARD_MPPTS];
static void *s_sense_callback_contexts[MAX_SOLAR_BOARD_MPPTS];
static uint8_t s_num_sense_callbacks;

StatusCode TEST_MOCK(sense_register)(SenseCallback callback, void *context) {
  s_sense_callbacks[s_num_sense_callbacks] = callback;
  s_sense_callback_contexts[s_num_sense_callbacks] = context;
  s_num_sense_callbacks++;
  return STATUS_CODE_OK;
}

static uint16_t s_mppt_current_ret;
static uint16_t s_mppt_voltage_ret;
static uint16_t s_mppt_pwm_ret;
static uint8_t s_mppt_status_ret;

static uint8_t s_mppt_current_pin;
static uint8_t s_mppt_voltage_pin;
static uint8_t s_mppt_pwm_pin;
static uint8_t s_mppt_status_pin;

StatusCode TEST_MOCK(mppt_read_current)(SpiPort port, uint16_t *current, uint8_t pin) {
  *current = s_mppt_current_ret;
  s_mppt_current_pin = pin;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(mppt_read_voltage_in)(SpiPort port, uint16_t *voltage, uint8_t pin) {
  *voltage = s_mppt_voltage_ret;
  s_mppt_voltage_pin = pin;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(mppt_read_pwm)(SpiPort port, uint16_t *pwm, uint8_t pin) {
  *pwm = s_mppt_pwm_ret;
  s_mppt_pwm_pin = pin;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(mppt_read_status)(SpiPort port, uint16_t *status, uint8_t pin) {
  *status = s_mppt_status_ret;
  s_mppt_status_pin = pin;
  return STATUS_CODE_OK;
}

static uint8_t s_num_faults_raised;
static EESolarFault s_faults_raised[MAX_FAULTS];
static uint8_t s_fault_data[MAX_FAULTS];

StatusCode TEST_MOCK(fault_handler_raise_fault)(EESolarFault fault, uint8_t fault_data) {
  TEST_ASSERT_MESSAGE(s_num_faults_raised < MAX_FAULTS, "Too many faults were raised to store!");
  s_faults_raised[s_num_faults_raised] = fault;
  s_fault_data[s_num_faults_raised] = fault_data;
  s_num_faults_raised++;
  return STATUS_CODE_OK;
}

static void prv_trigger_sense_cycle(void) {
  for (uint8_t i = 0; i < s_num_sense_callbacks; i++) {
    s_sense_callbacks[i](s_sense_callback_contexts[i]);
  }
}

void setup_test(void) {
  event_queue_init();
  gpio_init();
  mppt_init();
  data_store_init();
  // The dependency on SPI is mocked out, so we don't initialize it.

  s_num_sense_callbacks = 0;
  s_mppt_current_ret = TEST_CURRENT;
  s_mppt_voltage_ret = TEST_VOLTAGE;
  s_mppt_pwm_ret = TEST_PWM;
  s_mppt_status_ret = TEST_STATUS;
  s_mppt_current_pin = INVALID_MPPT;
  s_mppt_voltage_pin = INVALID_MPPT;
  s_mppt_pwm_pin = INVALID_MPPT;
  s_mppt_status_pin = INVALID_MPPT;

  s_num_faults_raised = 0;
}
void teardown_test(void) {}

// Test that all the values are set in a cycle with a single MPPT.
void test_single_mppt_cycle_sets_values(void) {
  bool is_set;
  uint32_t set_value;
  SenseMpptSettings settings = {
    .mppt_count = 1,
    .spi_port = TEST_SPI_PORT,
    .mppt_current_scaling_factor = 1.0f,
    .mppt_vin_scaling_factor = 1.0f,
  };
  TEST_ASSERT_OK(sense_mppt_init(&settings));

  // nothing should be set yet
  data_store_get_is_set(DATA_POINT_MPPT_CURRENT(0), &is_set);
  TEST_ASSERT_EQUAL(false, is_set);
  data_store_get_is_set(DATA_POINT_MPPT_VOLTAGE(0), &is_set);
  TEST_ASSERT_EQUAL(false, is_set);
  data_store_get_is_set(DATA_POINT_MPPT_PWM(0), &is_set);
  TEST_ASSERT_EQUAL(false, is_set);
  data_store_get_is_set(DATA_POINT_MPPT_STATUS(0), &is_set);
  TEST_ASSERT_EQUAL(false, is_set);

  // trigger the sense cycle, everything should then be set to the correct values
  prv_trigger_sense_cycle();

  data_store_get_is_set(DATA_POINT_MPPT_CURRENT(0), &is_set);
  TEST_ASSERT_EQUAL(true, is_set);
  data_store_get_is_set(DATA_POINT_MPPT_VOLTAGE(0), &is_set);
  TEST_ASSERT_EQUAL(true, is_set);
  data_store_get_is_set(DATA_POINT_MPPT_PWM(0), &is_set);
  TEST_ASSERT_EQUAL(true, is_set);
  data_store_get_is_set(DATA_POINT_MPPT_STATUS(0), &is_set);
  TEST_ASSERT_EQUAL(true, is_set);

  data_store_get(DATA_POINT_MPPT_CURRENT(0), &set_value);
  TEST_ASSERT_EQUAL(TEST_CURRENT, set_value);
  data_store_get(DATA_POINT_MPPT_VOLTAGE(0), &set_value);
  TEST_ASSERT_EQUAL(TEST_VOLTAGE, set_value);
  data_store_get(DATA_POINT_MPPT_PWM(0), &set_value);
  TEST_ASSERT_EQUAL(TEST_PWM, set_value);
  data_store_get(DATA_POINT_MPPT_STATUS(0), &set_value);
  TEST_ASSERT_EQUAL(TEST_STATUS, set_value);

  // the pins used should match with the MPPT
  TEST_ASSERT_EQUAL(0, s_mppt_current_pin);
  TEST_ASSERT_EQUAL(0, s_mppt_voltage_pin);
  TEST_ASSERT_EQUAL(0, s_mppt_pwm_pin);
  TEST_ASSERT_EQUAL(0, s_mppt_status_pin);

  // the default status is OK, so no errors should have been thrown
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that all the values are set in a cycle with the max number of MPPTs.
void test_max_mppt_cycle_sets_values(void) {
  bool is_set;
  uint32_t set_value;
  SenseMpptSettings settings = {
    .mppt_count = MAX_SOLAR_BOARD_MPPTS,
    .spi_port = TEST_SPI_PORT,
    .mppt_current_scaling_factor = 1.0f,
    .mppt_vin_scaling_factor = 1.0f,
  };
  TEST_ASSERT_OK(sense_mppt_init(&settings));

  // nothing should be set yet
  for (Mppt mppt = 0; mppt < MAX_SOLAR_BOARD_MPPTS; mppt++) {
    data_store_get_is_set(DATA_POINT_MPPT_CURRENT(mppt), &is_set);
    TEST_ASSERT_EQUAL(false, is_set);
    data_store_get_is_set(DATA_POINT_MPPT_VOLTAGE(mppt), &is_set);
    TEST_ASSERT_EQUAL(false, is_set);
    data_store_get_is_set(DATA_POINT_MPPT_PWM(mppt), &is_set);
    TEST_ASSERT_EQUAL(false, is_set);
    data_store_get_is_set(DATA_POINT_MPPT_STATUS(mppt), &is_set);
    TEST_ASSERT_EQUAL(false, is_set);
  }

  // trigger the sense cycle, everything should then be set to the correct values
  prv_trigger_sense_cycle();

  for (Mppt mppt = 0; mppt < MAX_SOLAR_BOARD_MPPTS; mppt++) {
    data_store_get_is_set(DATA_POINT_MPPT_CURRENT(mppt), &is_set);
    TEST_ASSERT_EQUAL(true, is_set);
    data_store_get_is_set(DATA_POINT_MPPT_VOLTAGE(mppt), &is_set);
    TEST_ASSERT_EQUAL(true, is_set);
    data_store_get_is_set(DATA_POINT_MPPT_PWM(mppt), &is_set);
    TEST_ASSERT_EQUAL(true, is_set);
    data_store_get_is_set(DATA_POINT_MPPT_STATUS(mppt), &is_set);
    TEST_ASSERT_EQUAL(true, is_set);

    data_store_get(DATA_POINT_MPPT_CURRENT(mppt), &set_value);
    TEST_ASSERT_EQUAL(TEST_CURRENT, set_value);
    data_store_get(DATA_POINT_MPPT_VOLTAGE(mppt), &set_value);
    TEST_ASSERT_EQUAL(TEST_VOLTAGE, set_value);
    data_store_get(DATA_POINT_MPPT_PWM(mppt), &set_value);
    TEST_ASSERT_EQUAL(TEST_PWM, set_value);
    data_store_get(DATA_POINT_MPPT_STATUS(mppt), &set_value);
    TEST_ASSERT_EQUAL(TEST_STATUS, set_value);
  }

  // the default status is OK, so no errors should have been thrown
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that we can trigger each fault event for each MPPT with the correct data.
void test_status_faults(void) {
  SenseMpptSettings settings = {
    .mppt_count = MAX_SOLAR_BOARD_MPPTS,
    .spi_port = TEST_SPI_PORT,
    .mppt_current_scaling_factor = 1.0f,
    .mppt_vin_scaling_factor = 1.0f,
  };
  TEST_ASSERT_OK(sense_mppt_init(&settings));

  // used to ensure that one event was raised per MPPT regardless of order
  uint8_t times_mppt_event_raised[MAX_SOLAR_BOARD_MPPTS] = { 0 };

  // overvoltage fault event
  s_mppt_status_ret = SPV1020_OVV_MASK;
  prv_trigger_sense_cycle();
  TEST_ASSERT_EQUAL(MAX_SOLAR_BOARD_MPPTS, s_num_faults_raised);
  for (Mppt i = 0; i < MAX_SOLAR_BOARD_MPPTS; i++) {
    TEST_ASSERT_EQUAL(EE_SOLAR_FAULT_MPPT_OVERVOLTAGE, s_faults_raised[i]);
    TEST_ASSERT(s_fault_data[i] < MAX_SOLAR_BOARD_MPPTS);
    times_mppt_event_raised[s_fault_data[i]]++;
  }
  // an event was raised for each MPPT
  for (Mppt mppt = 0; mppt < MAX_SOLAR_BOARD_MPPTS; mppt++) {
    TEST_ASSERT_EQUAL(1, times_mppt_event_raised[mppt]);
    times_mppt_event_raised[mppt] = 0;  // set up for the next fault
  }
  s_num_faults_raised = 0;  // reset for easier testing

  // overtemperature fault event
  s_mppt_status_ret = SPV1020_OVT_MASK;
  prv_trigger_sense_cycle();
  TEST_ASSERT_EQUAL(MAX_SOLAR_BOARD_MPPTS, s_num_faults_raised);
  for (Mppt i = 0; i < MAX_SOLAR_BOARD_MPPTS; i++) {
    TEST_ASSERT_EQUAL(EE_SOLAR_FAULT_MPPT_OVERTEMPERATURE, s_faults_raised[i]);
    TEST_ASSERT(s_fault_data[i] < MAX_SOLAR_BOARD_MPPTS);
    times_mppt_event_raised[s_fault_data[i]]++;
  }
  for (Mppt mppt = 0; mppt < MAX_SOLAR_BOARD_MPPTS; mppt++) {
    TEST_ASSERT_EQUAL(1, times_mppt_event_raised[mppt]);
    times_mppt_event_raised[mppt] = 0;  // set up for the next fault
  }
  s_num_faults_raised = 0;

  // overcurrent fault event
  s_mppt_status_ret = SPV1020_OVC_MASK;
  prv_trigger_sense_cycle();
  TEST_ASSERT_EQUAL(MAX_SOLAR_BOARD_MPPTS, s_num_faults_raised);
  for (Mppt i = 0; i < MAX_SOLAR_BOARD_MPPTS; i++) {
    TEST_ASSERT_EQUAL(EE_SOLAR_FAULT_MPPT_OVERCURRENT, s_faults_raised[i]);
    TEST_ASSERT_BITS_HIGH(0b11110000, s_fault_data[i]);  // all 4 branches set
    Mppt mppt = s_fault_data[i] & 0xF;
    TEST_ASSERT(mppt < MAX_SOLAR_BOARD_MPPTS);
    times_mppt_event_raised[mppt]++;
  }
  for (Mppt mppt = 0; mppt < MAX_SOLAR_BOARD_MPPTS; mppt++) {
    TEST_ASSERT_EQUAL(1, times_mppt_event_raised[mppt]);
    times_mppt_event_raised[mppt] = 0;  // set up for the next fault
  }
  s_num_faults_raised = 0;

  // all of them at the same time -  we check that the expected number of events per MPPT were
  // raised and that the expected number of each type of event was raised without checking order
  uint8_t num_ovv_events = 0;
  uint8_t num_ovt_events = 0;
  uint8_t num_ovc_events = 0;
  s_mppt_status_ret = SPV1020_OVV_MASK | SPV1020_OVT_MASK | SPV1020_OVC_MASK;
  prv_trigger_sense_cycle();
  TEST_ASSERT_EQUAL(3 * MAX_SOLAR_BOARD_MPPTS, s_num_faults_raised);

  for (uint8_t i = 0; i < 3 * MAX_SOLAR_BOARD_MPPTS; i++) {
    Mppt mppt = INVALID_MPPT;
    switch (s_faults_raised[i]) {
      case EE_SOLAR_FAULT_MPPT_OVERVOLTAGE:
        num_ovv_events++;
        mppt = s_fault_data[i];
        break;
      case EE_SOLAR_FAULT_MPPT_OVERTEMPERATURE:
        num_ovt_events++;
        mppt = s_fault_data[i];
        break;
      case EE_SOLAR_FAULT_MPPT_OVERCURRENT:
        num_ovc_events++;
        TEST_ASSERT_BITS_HIGH(0b11110000, s_fault_data[i]);  // all 4 branches set
        mppt = s_fault_data[i] & 0xF;
        break;
      default:
        TEST_FAIL_MESSAGE("Unknown fault event raised!\n");
        continue;
    }

    TEST_ASSERT(mppt < MAX_SOLAR_BOARD_MPPTS);
    times_mppt_event_raised[mppt]++;
  }
  s_num_faults_raised = 0;

  for (Mppt mppt = 0; mppt < MAX_SOLAR_BOARD_MPPTS; mppt++) {
    TEST_ASSERT_EQUAL(3, times_mppt_event_raised[mppt]);  // 3 distinct fault events
  }
  TEST_ASSERT_EQUAL(MAX_SOLAR_BOARD_MPPTS, num_ovv_events);
  TEST_ASSERT_EQUAL(MAX_SOLAR_BOARD_MPPTS, num_ovt_events);
  TEST_ASSERT_EQUAL(MAX_SOLAR_BOARD_MPPTS, num_ovc_events);
}

// Test that the current and vin scaling factors are respected.
void test_scaling_factor(void) {
  SenseMpptSettings settings = {
    .mppt_count = 1,
    .spi_port = TEST_SPI_PORT,
    .mppt_current_scaling_factor = 2.0f,
    .mppt_vin_scaling_factor = 0.5f,
  };
  TEST_ASSERT_OK(sense_mppt_init(&settings));

  s_mppt_current_ret = 7;
  s_mppt_voltage_ret = 7;

  prv_trigger_sense_cycle();

  uint32_t set_value = 0;
  data_store_get(DATA_POINT_MPPT_CURRENT(0), &set_value);
  TEST_ASSERT_EQUAL(14, set_value);
  data_store_get(DATA_POINT_MPPT_VOLTAGE(0), &set_value);
  TEST_ASSERT_EQUAL(3, set_value);  // truncates
}

// Test that initializing with invalid settings fails gracefully.
void test_invalid_settings(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sense_mppt_init(NULL));

  // too many MPPTs
  SenseMpptSettings invalid_settings = {
    .mppt_count = MAX_SOLAR_BOARD_MPPTS + 1,
    .spi_port = TEST_SPI_PORT,
    .mppt_current_scaling_factor = 1.0f,
    .mppt_vin_scaling_factor = 1.0f,
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sense_mppt_init(&invalid_settings));
}
