// Test sequence for passive balancing module
#include "passive_balance.h"

#include "bms_events.h"
#include "delay.h"
#include "interrupt.h"
#include "ltc_afe.h"
#include "ms_test_helpers.h"
#include "stdio.h"
#include "string.h"
#include "cell_sense.h"

#include "log.h"

static uint16_t s_cell_balanced = 0;
static bool s_cell_discharge = false;

// Just placeholders for afe settings from test_ltc_afe.c
#define TEST_LTC_AFE_ADC_MODE LTC_AFE_ADC_MODE_7KHZ
#define TEST_LTC_AFE_SPI_PORT SPI_PORT_1
#define TEST_LTC_AFE_SPI_BAUDRATE 750000
#define TEST_LTC_AFE_SPI_MOSI \
  { .port = GPIO_PORT_A, .pin = 7 }
#define TEST_LTC_AFE_SPI_MISO \
  { .port = GPIO_PORT_A, .pin = 6 }
#define TEST_LTC_AFE_SPI_SCLK \
  { .port = GPIO_PORT_A, .pin = 5 }
#define TEST_LTC_AFE_SPI_CS \
  { .port = GPIO_PORT_A, .pin = 4 }

StatusCode TEST_MOCK(ltc_afe_toggle_cell_discharge)(LtcAfeStorage *afe, uint16_t cell, bool discharge) {
  s_cell_balanced = cell;
  s_cell_discharge = discharge;
  return STATUS_CODE_OK;
}

// Number of cell voltages to be directly modified during testing
static const uint8_t NUM_TEST_VOLTAGES = 4;

static LtcAfeSettings s_test_afe_settings = {
  .num_cells = NUM_TOTAL_CELLS,
  .num_devices = NUM_AFES,
  .num_thermistors = LTC_AFE_MAX_THERMISTORS,
  .ltc_events = { .trigger_cell_conv_event = BMS_AFE_EVENT_TRIGGER_CELL_CONV,
                  .cell_conv_complete_event = BMS_AFE_EVENT_CELL_CONV_COMPLETE,
                  .trigger_aux_conv_event = BMS_AFE_EVENT_TRIGGER_AUX_CONV,
                  .aux_conv_complete_event = BMS_AFE_EVENT_AUX_CONV_COMPLETE,
                  .callback_run_event = BMS_AFE_EVENT_CALLBACK_RUN,
                  .fault_event = BMS_AFE_EVENT_FAULT },
  .cell_result_cb = NULL,
  .aux_result_cb = NULL,
  .result_context = NULL,
  .mosi = TEST_LTC_AFE_SPI_MOSI,
  .miso = TEST_LTC_AFE_SPI_MISO,
  .sclk = TEST_LTC_AFE_SPI_SCLK,
  .cs = TEST_LTC_AFE_SPI_CS,

  .spi_port = TEST_LTC_AFE_SPI_PORT,
  .spi_baudrate = TEST_LTC_AFE_SPI_BAUDRATE,
  .adc_mode = TEST_LTC_AFE_ADC_MODE,
};

static LtcAfeStorage s_test_afe_storage;

static const uint16_t AFE_DEFAULT_DISCHARGE_BITSET = 0;

void setup_test(void) {
  LOG_DEBUG("Setting up test sequence\n");
  interrupt_init();
  soft_timer_init();

  // Configure all cells for output
  for (uint8_t i = 0; i < LTC_AFE_MAX_DEVICES; i++) {
    s_test_afe_settings.cell_bitset[i] = 1;
    for (uint8_t j = 1; j < LTC_AFE_MAX_CELLS_PER_DEVICE; j++) {
      s_test_afe_settings.cell_bitset[i] <<= 1;
      s_test_afe_settings.cell_bitset[i] |= 0x1;
    }
  }

  ltc_afe_init(&s_test_afe_storage, &s_test_afe_settings);
  //TEST_ASSERT_OK(passive_balance_init(&s_test_afe_storage));
}

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
  test_voltages[0] = 1040;
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
