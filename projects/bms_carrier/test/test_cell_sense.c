#include "cell_sense.h"

#include "bms.h"
#include "bms_events.h"
#include "current_sense.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ltc_afe.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "test_helpers.h"

#define NUM_GOOD_CELL_SENSE_TRIALS 3
#define NUM_DEVICES_TO_TEST 1
#define NUM_CELLS_TO_TEST 4
#define NUM_THERMISTORS_TO_TEST 4

#define VOLTAGE_READING_MAX 1337
#define VOLTAGE_READING_MIN 42
#define TEMP_READING_MAX 1337
#define TEMP_READING_MIN 42
static AfeReadings s_raw_readings = {
  .voltages = { 42, 69, 420, 1337 },
  .temps = { 42, 69, 420, 1337 },
};
static AfeReadings s_readings;

static bool s_is_charging;
bool TEST_MOCK(current_sense_is_charging)() {
  return s_is_charging;
}

uint8_t s_expected_fault_bitset;
void TEST_MOCK(fault_bps_set)(uint8_t fault_bitmask) {
  TEST_ASSERT_EQUAL((s_expected_fault_bitset & fault_bitmask) == 0x0, false);
}

void TEST_MOCK(fault_bps_clear)(uint8_t fault_bitmask) {
  TEST_ASSERT_EQUAL((s_expected_fault_bitset & fault_bitmask) == 0x0, true);
}

static bool s_afe_should_fault;
bool TEST_MOCK(ltc_afe_process_event)(LtcAfeStorage *afe, const Event *e) {
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;
  LtcAfeResultCallback cb;
  uint16_t *result_arr;
  if (e->id == BMS_AFE_EVENT_TRIGGER_AUX_CONV) {
    cb = afe->settings.aux_result_cb;
    result_arr = s_raw_readings.temps;
  } else if (e->id == BMS_AFE_EVENT_TRIGGER_CELL_CONV) {
    cb = afe->settings.cell_result_cb;
    result_arr = s_raw_readings.voltages;
  } else {
    return false;
  }

  TEST_ASSERT_NOT_NULL(cb);
  if (s_afe_should_fault)
    return status_ok(event_raise_priority(EVENT_PRIORITY_HIGHEST, afe_events->fault_event, 0));
  cb(result_arr, afe->settings.num_cells, afe->settings.result_context);
  return status_ok(event_raise_priority(EVENT_PRIORITY_HIGHEST, afe_events->callback_run_event, 0));
}

static LtcAfeStorage s_afe;
static StatusCode prv_init_ltc(void) {
  status_ok_or_return(gpio_init());
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  // TODO(SOFT-9): Should update this to match hardware and add missing initializations
  LtcAfeSettings afe_settings = {
    .num_devices = NUM_DEVICES_TO_TEST,
    .num_cells = NUM_CELLS_TO_TEST,
    .num_thermistors = NUM_THERMISTORS_TO_TEST,
    // only need to initialize events so the correct ones are called
    .ltc_events = { .trigger_cell_conv_event = BMS_AFE_EVENT_TRIGGER_CELL_CONV,
                    .cell_conv_complete_event = BMS_AFE_EVENT_CELL_CONV_COMPLETE,
                    .trigger_aux_conv_event = BMS_AFE_EVENT_TRIGGER_AUX_CONV,
                    .aux_conv_complete_event = BMS_AFE_EVENT_AUX_CONV_COMPLETE,
                    .callback_run_event = BMS_AFE_EVENT_CALLBACK_RUN,
                    .fault_event = BMS_AFE_EVENT_FAULT },
    .cell_result_cb = NULL,
    .aux_result_cb = NULL,
    .result_context = NULL,
  };
  return ltc_afe_init(&s_afe, &afe_settings);
}

void prv_check_cell_results(bool is_clear) {
  for (size_t i = 0; i < NUM_CELLS_TO_TEST; i++) {
    if (is_clear) {
      TEST_ASSERT_EQUAL(s_readings.temps[i], 0);
      continue;
    }

    TEST_ASSERT_TRUE(s_readings.voltages[i] <= VOLTAGE_READING_MAX);
    TEST_ASSERT_TRUE(s_readings.voltages[i] >= VOLTAGE_READING_MIN);
    s_readings.voltages[i] = 0;
  }
}

void prv_check_temp_results(bool is_clear) {
  for (size_t i = 0; i < NUM_CELLS_TO_TEST; i++) {
    if (is_clear) {
      TEST_ASSERT_EQUAL(s_readings.temps[i], 0);
      continue;
    }

    TEST_ASSERT_TRUE(s_readings.temps[i] <= TEMP_READING_MAX);
    TEST_ASSERT_TRUE(s_readings.temps[i] >= TEMP_READING_MIN);
    s_readings.temps[i] = 0;
  }
}

void prv_test_fsm_fault() {
  Event e = { 0 };
  LOG_DEBUG("Verifying cell result empty\n");
  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, BMS_AFE_EVENT_TRIGGER_CELL_CONV);
  ltc_afe_process_event(&s_afe, &e);
  prv_check_cell_results(true);

  LOG_DEBUG("Verifying FSM result fault\n");
  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, BMS_AFE_EVENT_FAULT);
  cell_sense_process_event(&e);
}

void prv_test_single_loop() {
  Event e = { 0 };
  LOG_DEBUG("Verifying if cell fault state matches\n");
  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, BMS_AFE_EVENT_TRIGGER_CELL_CONV);
  ltc_afe_process_event(&s_afe, &e);
  prv_check_cell_results(false);

  LOG_DEBUG("Verifying no FSM result fault\n");
  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, BMS_AFE_EVENT_CALLBACK_RUN);
  cell_sense_process_event(&e);

  // Verify aux fault state matches
  LOG_DEBUG("Verifying if aux fault state matches\n");
  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, BMS_AFE_EVENT_TRIGGER_AUX_CONV);
  ltc_afe_process_event(&s_afe, &e);
  prv_check_temp_results(false);

  LOG_DEBUG("Verifying no FSM result fault\n");
  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, BMS_AFE_EVENT_CALLBACK_RUN);
  cell_sense_process_event(&e);
}

void setup_test(void) {
  s_is_charging = false;
  s_afe_should_fault = false;
  s_expected_fault_bitset = EE_BPS_STATE_OK;
  TEST_ASSERT_OK(prv_init_ltc());
}

void teardown_test(void) {}

void test_normal_cell_sense(void) {
  CellSenseSettings settings = {
    .undervoltage_dmv = VOLTAGE_READING_MIN,
    .overvoltage_dmv = VOLTAGE_READING_MAX,
    .charge_overtemp_dmv = TEMP_READING_MAX,
    .discharge_overtemp_dmv = TEMP_READING_MAX,
  };
  Event e = { 0 };
  TEST_ASSERT_OK(cell_sense_init(&settings, &s_readings, &s_afe));
  s_expected_fault_bitset = EE_BPS_STATE_OK;
  for (size_t i = 0; i < NUM_GOOD_CELL_SENSE_TRIALS; i++) {
    prv_test_single_loop();
  }
  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, BMS_AFE_EVENT_TRIGGER_CELL_CONV);
}

void test_cell_undervoltage_fault_cell_sense(void) {
  CellSenseSettings settings = {
    .undervoltage_dmv = VOLTAGE_READING_MIN + 50,  // Some voltages should now be undervoltage
    .overvoltage_dmv = VOLTAGE_READING_MAX,
    .charge_overtemp_dmv = TEMP_READING_MAX,
    .discharge_overtemp_dmv = TEMP_READING_MAX,
  };
  Event e = { 0 };
  TEST_ASSERT_OK(cell_sense_init(&settings, &s_readings, &s_afe));

  s_expected_fault_bitset = EE_BPS_STATE_FAULT_AFE_CELL;
  prv_test_single_loop();
  // Verify loop is still occuring
  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, BMS_AFE_EVENT_TRIGGER_CELL_CONV);
}

void test_cell_overvoltage_fault_cell_sense(void) {
  CellSenseSettings settings = {
    .undervoltage_dmv = VOLTAGE_READING_MIN,
    .overvoltage_dmv = VOLTAGE_READING_MAX - 50,
    .charge_overtemp_dmv = TEMP_READING_MAX,
    .discharge_overtemp_dmv = TEMP_READING_MAX,
  };
  Event e = { 0 };
  TEST_ASSERT_OK(cell_sense_init(&settings, &s_readings, &s_afe));
  s_expected_fault_bitset = EE_BPS_STATE_FAULT_AFE_CELL;
  prv_test_single_loop();
  // Verify loop is still occuring
  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, BMS_AFE_EVENT_TRIGGER_CELL_CONV);
}

void test_temp_charging_fault_cell_sense(void) {
  CellSenseSettings settings = {
    .undervoltage_dmv = VOLTAGE_READING_MIN,
    .overvoltage_dmv = VOLTAGE_READING_MAX,
    .charge_overtemp_dmv = TEMP_READING_MAX - 50,
    .discharge_overtemp_dmv = TEMP_READING_MAX,
  };
  Event e = { 0 };
  TEST_ASSERT_OK(cell_sense_init(&settings, &s_readings, &s_afe));

  s_is_charging = false;
  s_expected_fault_bitset = EE_BPS_STATE_OK;
  prv_test_single_loop();
  s_is_charging = true;
  s_expected_fault_bitset = EE_BPS_STATE_FAULT_AFE_TEMP;
  prv_test_single_loop();

  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, BMS_AFE_EVENT_TRIGGER_CELL_CONV);
}

void test_temp_discharging_fault_cell_sense(void) {
  CellSenseSettings settings = {
    .undervoltage_dmv = VOLTAGE_READING_MIN,
    .overvoltage_dmv = VOLTAGE_READING_MAX,
    .charge_overtemp_dmv = TEMP_READING_MAX,
    .discharge_overtemp_dmv = TEMP_READING_MAX - 50,
  };
  Event e = { 0 };
  TEST_ASSERT_OK(cell_sense_init(&settings, &s_readings, &s_afe));

  s_is_charging = false;
  s_expected_fault_bitset = EE_BPS_STATE_FAULT_AFE_TEMP;
  prv_test_single_loop();
  LOG_DEBUG("RUNNING SECOND TEST\n");
  s_is_charging = true;
  s_expected_fault_bitset = EE_BPS_STATE_OK;
  prv_test_single_loop();

  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, BMS_AFE_EVENT_TRIGGER_CELL_CONV);
}

void test_afe_fsm_fault_cell_sense(void) {
  CellSenseSettings settings = {
    .undervoltage_dmv = VOLTAGE_READING_MIN,
    .overvoltage_dmv = VOLTAGE_READING_MAX,
    .charge_overtemp_dmv = TEMP_READING_MAX,
    .discharge_overtemp_dmv = TEMP_READING_MAX - 50,
  };
  Event e = { 0 };
  TEST_ASSERT_OK(cell_sense_init(&settings, &s_readings, &s_afe));
  s_afe_should_fault = true;

  s_expected_fault_bitset = EE_BPS_STATE_OK;
  for (size_t i = 0; i < MAX_AFE_FAULTS; i++) {
    prv_test_fsm_fault();
  }

  s_expected_fault_bitset = EE_BPS_STATE_FAULT_AFE_FSM;
  for (size_t i = 0; i < MAX_AFE_FAULTS; i++) {
    prv_test_fsm_fault();
  }

  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, BMS_AFE_EVENT_TRIGGER_CELL_CONV);
}
