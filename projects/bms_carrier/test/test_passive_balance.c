// Test sequence for passive balancing module
#include "passive_balance.h"

#include "cell_sense.h"
#include "ltc_afe.h"
#include "ms_test_helpers.h"

#include "log.h"

#define NUM_TEST_CELLS 4

static uint16_t s_cell_balanced = 0;

StatusCode TEST_MOCK(ltc_afe_toggle_cell_discharge)(LtcAfeStorage *afe, uint16_t cell,
                                                    bool discharge) {
  if (discharge) {
    s_cell_balanced = cell;
  }
  return STATUS_CODE_OK;
}

// To store voltages for test
static uint16_t s_test_voltages[NUM_TEST_CELLS];

static LtcAfeStorage s_test_afe_storage = {
  .settings = {
    .num_devices = 1,
    .num_cells = NUM_TEST_CELLS,
  },
};

// Set all voltages to 35000.  Indices 0-3 only used in most tests for simplicity
void setup_test(void) {
  for (uint8_t i = 0; i < NUM_TEST_CELLS; i++) {
    s_test_voltages[i] = 35000;
  }
  s_cell_balanced = 0xFFFF;
}

void teardown_test(void) {}

// Balance a few values, ensure cell bitset is set properly.
void test_normal_operation(void) {
  LOG_DEBUG("Testing passive balancing normal operation\n");
  // Stores cell and device that should be balanced
  uint8_t balance_cell = 2;

  // Only cell 2 should be balanced
  s_test_voltages[0] = 35000;
  s_test_voltages[1] = 35000;
  s_test_voltages[balance_cell] = 36000;
  s_test_voltages[3] = 35000;

  // Verify
  passive_balance(s_test_voltages, NUM_TEST_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(s_cell_balanced, balance_cell);
}

// First cell in range
void test_balance_first_cell(void) {
  LOG_DEBUG("Testing balancing first cell in range\n");
  uint8_t balance_cell = 0;
  s_cell_balanced = 0xFFFF;

  // Only cell 0 should be balanced
  s_test_voltages[balance_cell] = 39000;
  s_test_voltages[1] = 35000;
  s_test_voltages[2] = 35000;
  s_test_voltages[3] = 35000;

  // Verify
  passive_balance(s_test_voltages, NUM_TEST_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(s_cell_balanced, balance_cell);
}

// Last cell in range
void test_balance_last_cell(void) {
  LOG_DEBUG("Testing balancing last cell in range\n");
  uint8_t balance_cell = NUM_TEST_CELLS - 1;
  s_cell_balanced = 0;

  // Only last cell should be balanced
  s_test_voltages[0] = 35000;
  s_test_voltages[1] = 35000;
  s_test_voltages[2] = 35000;
  s_test_voltages[3] = 35000;
  s_test_voltages[balance_cell] = 38000;

  // Verify
  passive_balance(s_test_voltages, NUM_TEST_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(s_cell_balanced, balance_cell);
}

// Test cell 2 at edge of PASSIVE_BALANCE_MIN_VOLTAGE_DIFF_MV
void test_balance_range(void) {
  // Same general procedures as in tests above
  LOG_DEBUG("Testing balancing around edge of range\n");

  uint8_t balance_cell = 0;

  // No cell should be balanced
  s_test_voltages[0] = 35000;
  s_test_voltages[1] = 35000;
  s_test_voltages[2] = 35000 + (PASSIVE_BALANCE_MIN_VOLTAGE_DIFF_MV * 10) - 1;
  s_test_voltages[3] = 35000;

  // Verify
  passive_balance(s_test_voltages, NUM_TEST_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(0xFFFF, s_cell_balanced);

  balance_cell = 2;

  // Increment cell 2 voltage, should be balanced now
  s_test_voltages[balance_cell]++;

  // Verify
  passive_balance(s_test_voltages, NUM_TEST_CELLS, &s_test_afe_storage);
  TEST_ASSERT_EQUAL(s_cell_balanced, balance_cell);
}
