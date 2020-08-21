#include "fault_monitor.h"

#include <stdbool.h>

#include "data_store.h"
#include "exported_enums.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "solar_events.h"
#include "test_helpers.h"
#include "unity.h"

#define MAX_FAULTS 32

#define TEST_OUTPUT_OVERCURRENT_THRESHOLD 1000000000L
#define TEST_OUTPUT_OVERVOLTAGE_THRESHOLD 1000000000uL
#define TEST_TEMPERATURE_THRESHOLD 1000000000uL

#define TEST_ASSERT_NO_FAULT() TEST_ASSERT_EQUAL(0, s_num_faults_raised)

#define TEST_ASSERT_SINGLE_FAULT(fault, data)       \
  ({                                                \
    TEST_ASSERT_EQUAL(1, s_num_faults_raised);      \
    TEST_ASSERT_EQUAL((fault), s_faults_raised[0]); \
    TEST_ASSERT_EQUAL((data), s_fault_data[0]);     \
    s_num_faults_raised = 0;                        \
  })

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

static const Event s_data_ready_event = { .id = DATA_READY_EVENT };

static const FaultMonitorSettings s_base_settings = {
  .output_overcurrent_threshold_uA = TEST_OUTPUT_OVERCURRENT_THRESHOLD,
  .output_overvoltage_threshold_mV = TEST_OUTPUT_OVERVOLTAGE_THRESHOLD,
  .overtemperature_threshold_dC = TEST_TEMPERATURE_THRESHOLD,
};

static void prv_initialize(SolarMpptCount mppt_count) {
  FaultMonitorSettings settings = s_base_settings;
  settings.mppt_count = mppt_count;
  TEST_ASSERT_OK(fault_monitor_init(&settings));
}

void setup_test(void) {
  data_store_init();
}
void teardown_test(void) {}

// Test that no faults are checked for when no data points are set.
void test_nothing_set(void) {
  prv_initialize(SOLAR_BOARD_6_MPPTS);
  fault_monitor_process_event(&s_data_ready_event);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// A template test for a simple threshold.
static void prv_test_basic_threshold_fault(DataPoint data_point, uint32_t threshold,
                                           EESolarFault fault, uint16_t fault_data) {
  prv_initialize(SOLAR_BOARD_6_MPPTS);

  data_store_set(data_point, 1);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_NO_FAULT();

  data_store_set(data_point, threshold - 1);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_NO_FAULT();

  data_store_set(data_point, threshold + 1);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_SINGLE_FAULT(fault, fault_data);

  // fault must be tripped on the threshold
  data_store_set(data_point, threshold);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_SINGLE_FAULT(fault, fault_data);
}

// OUTPUT CURRENT FAULTS

// Test that the overcurrent threshold works.
void test_output_overcurrent_fault(void) {
  prv_test_basic_threshold_fault(DATA_POINT_CURRENT, TEST_OUTPUT_OVERCURRENT_THRESHOLD,
                                 EE_SOLAR_FAULT_OVERCURRENT, 0);
}

// Test that the fault on negative current works.
void test_output_current_negative_fault(void) {
  prv_initialize(SOLAR_BOARD_5_MPPTS);

  // positive currents were tested in the overcurrent test, but so this unit test can stand alone
  // we test again here
  data_store_set(DATA_POINT_CURRENT, 1);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_NO_FAULT();

  // if signed handling is incorrect, a EE_SOLAR_FAULT_OVERCURRENT will be raised here
  data_store_set(DATA_POINT_CURRENT, (uint32_t)-1);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_SINGLE_FAULT(EE_SOLAR_FAULT_NEGATIVE_CURRENT, 0);

  data_store_set(DATA_POINT_CURRENT, (uint32_t)-TEST_OUTPUT_OVERCURRENT_THRESHOLD);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_SINGLE_FAULT(EE_SOLAR_FAULT_NEGATIVE_CURRENT, 0);

  // must be strictly negative
  data_store_set(DATA_POINT_CURRENT, 0);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_NO_FAULT();
}

// OUTPUT VOLTAGE FAULT

// Test that the basic overvoltage threshold works for the given number of MPPTs.
static void prv_test_overvoltage_fault_basic(SolarMpptCount mppt_count) {
  prv_initialize(mppt_count);

  // sum is |mppt_count|
  for (Mppt mppt = 0; mppt < mppt_count; mppt++) {
    data_store_set(DATA_POINT_VOLTAGE(mppt), 1);
  }
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_NO_FAULT();

  // sum is TEST_OUTPUT_OVERVOLTAGE_THRESHOLD - 1
  data_store_set(DATA_POINT_VOLTAGE(0), TEST_OUTPUT_OVERVOLTAGE_THRESHOLD - mppt_count);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_NO_FAULT();

  // sum is TEST_OUTPUT_OVERVOLTAGE_THRESHOLD + 1
  data_store_set(DATA_POINT_VOLTAGE(0), TEST_OUTPUT_OVERVOLTAGE_THRESHOLD - mppt_count + 2);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_SINGLE_FAULT(EE_SOLAR_FAULT_OVERVOLTAGE, 0);

  // sum is TEST_OUTPUT_OVERVOLTAGE_THRESHOLD, must be tripped on threshold
  data_store_set(DATA_POINT_VOLTAGE(0), TEST_OUTPUT_OVERVOLTAGE_THRESHOLD - mppt_count + 1);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_SINGLE_FAULT(EE_SOLAR_FAULT_OVERVOLTAGE, 0);
}

void test_overvoltage_fault_basic_6_mppt(void) {
  prv_test_overvoltage_fault_basic(SOLAR_BOARD_6_MPPTS);
}

void test_overvoltage_fault_basic_5_mppt(void) {
  prv_test_overvoltage_fault_basic(SOLAR_BOARD_5_MPPTS);

  // make sure it's definitely not checking the 6th MPPT
  for (Mppt mppt = 0; mppt < SOLAR_BOARD_5_MPPTS; mppt++) {
    data_store_set(DATA_POINT_VOLTAGE(mppt), 0);
  }
  data_store_set(DATA_POINT_VOLTAGE(SOLAR_BOARD_6_MPPTS - 1),
                 TEST_OUTPUT_OVERVOLTAGE_THRESHOLD + 1);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_NO_FAULT();
}

// Test that the overvoltage fault is still triggered if some voltages are unset.
void test_overvoltage_fault_some_unset(void) {
  prv_initialize(SOLAR_BOARD_6_MPPTS);

  // only first set
  data_store_set(DATA_POINT_VOLTAGE(0), TEST_OUTPUT_OVERVOLTAGE_THRESHOLD + 1);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_SINGLE_FAULT(EE_SOLAR_FAULT_OVERVOLTAGE, 0);

  // only first and last set
  data_store_set(DATA_POINT_VOLTAGE(0), 0);
  data_store_set(DATA_POINT_VOLTAGE(SOLAR_BOARD_6_MPPTS - 1),
                 TEST_OUTPUT_OVERVOLTAGE_THRESHOLD - 1);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_NO_FAULT();

  data_store_set(DATA_POINT_VOLTAGE(0), 2);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_SINGLE_FAULT(EE_SOLAR_FAULT_OVERVOLTAGE, 0);
}

// Test that the overvoltage fault is still triggered if adding the voltages would cause overflow.
void test_overvoltage_fault_overflow(void) {
  Event e = { 0 };
  uint32_t max_uint32 = 0xFFFFFFFF;
  prv_initialize(SOLAR_BOARD_6_MPPTS);

  // the value would become -1 if signed ints are used
  data_store_set(DATA_POINT_VOLTAGE(0), max_uint32);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_SINGLE_FAULT(EE_SOLAR_FAULT_OVERVOLTAGE, 0);

  // a naive sum would get 1 + max_uint32 == 0
  data_store_set(DATA_POINT_VOLTAGE(0), 1);
  data_store_set(DATA_POINT_VOLTAGE(1), max_uint32);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_SINGLE_FAULT(EE_SOLAR_FAULT_OVERVOLTAGE, 0);

  data_store_set(DATA_POINT_VOLTAGE(1), TEST_OUTPUT_OVERVOLTAGE_THRESHOLD - 2);
  data_store_set(DATA_POINT_VOLTAGE(2), max_uint32);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_SINGLE_FAULT(EE_SOLAR_FAULT_OVERVOLTAGE, 0);
}

// TEMPERATURE FAULT

// Test that the overtemperature threshold works.
void test_overtemperature_fault(void) {
  for (Mppt thermistor = 0; thermistor < SOLAR_BOARD_6_MPPTS; thermistor++) {
    // log so we can differentiate errors in the helper function
    LOG_DEBUG("Testing faults for thermistor %d\n", thermistor);
    data_store_init();  // to reset data from the last thermistor's test
    prv_test_basic_threshold_fault(DATA_POINT_TEMPERATURE(thermistor), TEST_TEMPERATURE_THRESHOLD,
                                   EE_SOLAR_FAULT_OVERTEMPERATURE, thermistor);
  }
}

// Test that we don't check the 6th MPPT's temperature on the 5 MPPT board.
void test_overtemperature_5_mppt_board(void) {
  prv_initialize(SOLAR_BOARD_5_MPPTS);
  data_store_set(DATA_POINT_TEMPERATURE(SOLAR_BOARD_6_MPPTS - 1), TEST_TEMPERATURE_THRESHOLD + 1);
  fault_monitor_process_event(&s_data_ready_event);
  TEST_ASSERT_NO_FAULT();
}

// ALL OF THEM

// Test that we can trip all the (non-mutually-exclusive) faults simultaneously.
void test_all_faults_simultaneously(void) {
  const SolarMpptCount mppt_count = SOLAR_BOARD_6_MPPTS;
  prv_initialize(mppt_count);

  data_store_set(DATA_POINT_CURRENT, TEST_OUTPUT_OVERCURRENT_THRESHOLD + 1);

  for (Mppt mppt = 0; mppt < mppt_count - 1; mppt++) {
    data_store_set(DATA_POINT_VOLTAGE(mppt), 1);
  }
  data_store_set(DATA_POINT_VOLTAGE(mppt_count - 1),
                 TEST_OUTPUT_OVERVOLTAGE_THRESHOLD - mppt_count + 2);

  for (Mppt mppt = 0; mppt < mppt_count; mppt++) {
    data_store_set(DATA_POINT_TEMPERATURE(mppt), TEST_TEMPERATURE_THRESHOLD + 1);
  }

  fault_monitor_process_event(&s_data_ready_event);

  // build a checklist of faults to see
  EESolarFault faults_wanted[MAX_SOLAR_BOARD_MPPTS + 2] = { 0 };
  uint8_t fault_data_wanted[MAX_SOLAR_BOARD_MPPTS + 2] = { 0 };
  bool faults_seen[MAX_SOLAR_BOARD_MPPTS + 2] = { false };
  uint8_t num_faults_wanted = 0;
  for (Mppt mppt = 0; mppt < mppt_count; mppt++) {
    faults_wanted[num_faults_wanted] = EE_SOLAR_FAULT_OVERTEMPERATURE;
    fault_data_wanted[num_faults_wanted] = mppt;
    num_faults_wanted++;
  }
  faults_wanted[num_faults_wanted] = EE_SOLAR_FAULT_OVERCURRENT;
  fault_data_wanted[num_faults_wanted] = 0;
  num_faults_wanted++;
  faults_wanted[num_faults_wanted] = EE_SOLAR_FAULT_OVERVOLTAGE;
  fault_data_wanted[num_faults_wanted] = 0;
  num_faults_wanted++;

  // check off the checklist
  for (uint8_t fault = 0; fault < s_num_faults_raised; fault++) {
    bool found = false;
    for (uint8_t i = 0; i < num_faults_wanted; i++) {
      if (s_faults_raised[fault] == faults_wanted[i] &&
          s_fault_data[fault] == fault_data_wanted[i]) {
        TEST_ASSERT_FALSE_MESSAGE(faults_seen[i], "Event seen multiple times");
        faults_seen[i] = true;
        found = true;
        break;
      }
    }
    TEST_ASSERT_TRUE_MESSAGE(found, "Unexpected event seen!");
  }
}

// EXTRA

// Test that |fault_monitor_process_event| returns whether it was passed a data ready event
// since that's when it processes the event, and handles null gracefully.
void test_fault_monitor_process_event_return_value(void) {
  prv_initialize(SOLAR_BOARD_6_MPPTS);
  TEST_ASSERT_TRUE(fault_monitor_process_event(&s_data_ready_event));
  Event non_data_ready_event = { .id = DATA_READY_EVENT + 1 };
  TEST_ASSERT_FALSE(fault_monitor_process_event(&non_data_ready_event));
  TEST_ASSERT_FALSE(fault_monitor_process_event(NULL));
}

// Test that null settings and settings with an MPPT count too large are handled gracefully.
void test_invalid_initialization(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, fault_monitor_init(NULL));
  FaultMonitorSettings invalid_settings = s_base_settings;
  invalid_settings.mppt_count = MAX_SOLAR_BOARD_MPPTS + 1;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, fault_monitor_init(&invalid_settings));
}
