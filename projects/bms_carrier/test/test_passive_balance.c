// Test sequence for passive balancing module
#include "passive_balance.h"

#include "cell_sense.h"
#include "ltc_afe.h"
#include "ms_test_helpers.h"

#include "log.h"

static uint16_t s_cell_balanced = 0;
static bool s_cell_discharge = false;

StatusCode TEST_MOCK(ltc_afe_toggle_cell_discharge)(LtcAfeStorage *afe, uint16_t cell,
                                                    bool discharge) {
  s_cell_balanced = cell;
  s_cell_discharge = discharge;
  return STATUS_CODE_OK;
}

// Number of cell voltages to be directly modified during testing
static const uint8_t NUM_TEST_VOLTAGES = 4;

static LtcAfeStorage s_test_afe_storage;

static const uint16_t AFE_DEFAULT_DISCHARGE_BITSET = 0;

void setup_test(void) {}

void teardown_test(void) {}

// Balance a few values, ensure cell bitset is set properly.
void test_normal_operation(void) {
  LOG_DEBUG("Testing passive balancing normal operation\n");
  // Doing all tests on first 4 cells and last cell for clarity, since max number of
  // devices is variable
  // Fill rest with numbers from 1010 to 1020
  uint16_t test_voltages[NUM_TOTAL_CELLS];
  for (uint8_t i = NUM_TEST_VOLTAGES; i < NUM_TOTAL_CELLS; i++) {
    test_voltages[i] = 1010;
  }

  // Stores cell and device that should be balanced
  uint8_t balance_cell = 2;

  // Only cell 2 should be balanced
  test_voltages[0] = 1000;
  test_voltages[1] = 1025;
  test_voltages[balance_cell] = 1030;
  test_voltages[3] = 1010;

  // Verify
  passive_balance(test_voltages, NUM_TOTAL_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(s_cell_balanced, balance_cell);
  TEST_ASSERT_EQUAL(s_cell_discharge, true);

  // First cell in range
  balance_cell = 0;

  // Only cell 0 should be balanced
  test_voltages[balance_cell] = 1040;
  test_voltages[1] = 1025;
  test_voltages[2] = 1010;
  test_voltages[3] = 1010;

  // Verify
  passive_balance(test_voltages, NUM_TOTAL_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(s_cell_balanced, balance_cell);
  TEST_ASSERT_EQUAL(s_cell_discharge, true);

  // Last cell in range
  balance_cell = NUM_TOTAL_CELLS - 1;

  // Only last cell should be balanced
  test_voltages[0] = 1000;
  test_voltages[1] = 1025;
  test_voltages[2] = 1030;
  test_voltages[balance_cell] = 1040;

  // Verify
  passive_balance(test_voltages, NUM_TOTAL_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(s_cell_balanced, balance_cell);
  TEST_ASSERT_EQUAL(s_cell_discharge, true);
}

// Balance values within allowed range, ensure they don't change
void test_balance_range(void) {
  LOG_DEBUG("Testing balancing around edge of range\n");
  // Same general procedures as above

  // Fill cells
  uint16_t test_voltages[NUM_TOTAL_CELLS];
  for (uint8_t i = NUM_TEST_VOLTAGES; i < NUM_TOTAL_CELLS; i++) {
    test_voltages[i] = 1010;
  }

  // Storage variables
  uint8_t balance_cell = 0;

  // No cell should be balanced
  s_cell_discharge = true;
  test_voltages[0] = 1000;
  test_voltages[1] = 1024;
  test_voltages[2] = 1000 + PASSIVE_BALANCE_MIN_VOLTAGE_DIFF_MV - 1;
  test_voltages[3] = 1010;

  // Verify
  passive_balance(test_voltages, NUM_TOTAL_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(s_cell_discharge, false);

  // Cell 2 should be balanced here
  balance_cell = 2;
  test_voltages[balance_cell] = 1000 + PASSIVE_BALANCE_MIN_VOLTAGE_DIFF_MV;

  // Verify
  passive_balance(test_voltages, NUM_TOTAL_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(s_cell_balanced, balance_cell);
  TEST_ASSERT_EQUAL(s_cell_discharge, true);
}
