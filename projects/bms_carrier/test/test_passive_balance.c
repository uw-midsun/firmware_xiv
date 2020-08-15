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

// To store voltages for test
static uint16_t s_test_voltages[NUM_TOTAL_CELLS];

static LtcAfeStorage s_test_afe_storage;

// Set all voltages to 1010.  Indices 0-3 only used in most tests for simplicity
void setup_test(void) {
  for (uint8_t i = 0; i < NUM_TOTAL_CELLS; i++) {
    s_test_voltages[i] = 1010;
  }
}

void teardown_test(void) {}

// Balance a few values, ensure cell bitset is set properly.
void test_normal_operation(void) {
  LOG_DEBUG("Testing passive balancing normal operation\n");
  // Stores cell and device that should be balanced
  uint8_t balance_cell = 2;

  // Only cell 2 should be balanced
  s_test_voltages[0] = 1000;
  s_test_voltages[1] = 1025;
  s_test_voltages[balance_cell] = 1030;
  s_test_voltages[3] = 1010;

  // Verify
  passive_balance(s_test_voltages, NUM_TOTAL_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(s_cell_balanced, balance_cell);
  TEST_ASSERT_EQUAL(s_cell_discharge, true);
}

// First cell in range
void test_balance_first_cell(void) {
  LOG_DEBUG("Testing balancing first cell in range\n");
  uint8_t balance_cell = 0;
  s_cell_balanced = 1;
  s_cell_discharge = false;

  // Only cell 0 should be balanced
  s_test_voltages[balance_cell] = 1040;
  s_test_voltages[1] = 1025;
  s_test_voltages[2] = 1010;
  s_test_voltages[3] = 1010;

  // Verify
  passive_balance(s_test_voltages, NUM_TOTAL_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(s_cell_balanced, balance_cell);
  TEST_ASSERT_EQUAL(s_cell_discharge, true);
}

// Last cell in range
void test_balance_last_cell(void) {
  LOG_DEBUG("Testing balancing last cell in range\n");
  uint8_t balance_cell = NUM_TOTAL_CELLS - 1;
  s_cell_balanced = 0;
  s_cell_discharge = false;

  // Only last cell should be balanced
  s_test_voltages[0] = 1000;
  s_test_voltages[1] = 1025;
  s_test_voltages[2] = 1030;
  s_test_voltages[3] = 1010;
  s_test_voltages[balance_cell] = 1040;

  // Verify
  passive_balance(s_test_voltages, NUM_TOTAL_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(s_cell_balanced, balance_cell);
  TEST_ASSERT_EQUAL(s_cell_discharge, true);
}

// Test cell 2 at edge of PASSIVE_BALANCE_MIN_VOLTAGE_DIFF_MV
void test_balance_range(void) {
  // Same general procedures as in tests above
  LOG_DEBUG("Testing balancing around edge of range\n");

  uint8_t balance_cell = 0;
  s_cell_discharge = false;

  // No cell should be balanced
  s_test_voltages[0] = 1000;
  s_test_voltages[1] = 1024;
  s_test_voltages[2] = 1000 + PASSIVE_BALANCE_MIN_VOLTAGE_DIFF_MV - 1;
  s_test_voltages[3] = 1010;

  // Verify
  passive_balance(s_test_voltages, NUM_TOTAL_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(s_cell_discharge, false);

  balance_cell = 2;
  s_cell_balanced = 0;
  s_cell_discharge = false;

  // Increment cell 2 voltage, should be balanced now
  s_test_voltages[balance_cell]++;

  // Verify
  passive_balance(s_test_voltages, NUM_TOTAL_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(s_cell_balanced, balance_cell);
  TEST_ASSERT_EQUAL(s_cell_discharge, true);
}
