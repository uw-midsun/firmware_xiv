// Test sequence for passive balancing module
#include "passive_balance.h"

#include "delay.h"
#include "interrupt.h"
#include "ms_test_helpers.h"
#include "ltc_afe.h"
#include "bms_events.h"
#include "stdio.h"
#include "string.h"

#include "log.h"

// Just placeholders for afe settings
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

// Number of voltages to be changed for testing
static const uint8_t NUM_TEST_VOLTAGES = 4;

static LtcAfeSettings test_afe_settings = {
  .num_cells = LTC_AFE_MAX_CELLS,
  .num_devices = LTC_AFE_MAX_DEVICES,
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
  //literally random values
  .mosi = TEST_LTC_AFE_SPI_MOSI,
  .miso = TEST_LTC_AFE_SPI_MISO,
  .sclk = TEST_LTC_AFE_SPI_SCLK,
  .cs = TEST_LTC_AFE_SPI_CS,

  .spi_port = TEST_LTC_AFE_SPI_PORT,
  .spi_baudrate = TEST_LTC_AFE_SPI_BAUDRATE,
  .adc_mode = TEST_LTC_AFE_ADC_MODE,
}; 

static LtcAfeStorage test_afe_storage;

static const uint16_t AFE_DEFAULT_DISCHARGE_BITSET = 0;

void setup_test(void) {
  LOG_DEBUG("Setting up test sequence\n");
  interrupt_init();
  soft_timer_init();

  // Configure all cells for output
  for(uint8_t i = 0; i < LTC_AFE_MAX_DEVICES; i++) {
    test_afe_settings.cell_bitset[i] = 1;
    for(uint8_t j = 1; j < LTC_AFE_MAX_CELLS_PER_DEVICE; j++) {
      test_afe_settings.cell_bitset[i] <<= 1;
      test_afe_settings.cell_bitset[i] |= 0x1;
    }
  }
  ltc_afe_init(&test_afe_storage, &test_afe_settings); 
  TEST_ASSERT_OK(passive_balance_init(&test_afe_storage));
}

void teardown_test(void) {}

// Balance a few values, ensure cell bitset is set properly.
void test_normal_operation(void) {
  LOG_DEBUG("Testing passive balancing normal operation\n");
  // Doing all tests on first 4 cells of device 0 or last cell for clarity, since max number of devices is variable 
  // Fill rest with numbers from 1010 to 1020
  for(uint8_t i = NUM_TEST_VOLTAGES; i < LTC_AFE_MAX_CELLS; i++) {
    test_afe_storage.cell_voltages[i] = 1010;
  }
  
  // Stores cell and device that should be balanced
  uint8_t balance_cell = 2;
  uint8_t balance_device = 0;

  // Remove this after rewriting passive_balance.c
  // If this is still here when I push I can't read
  SoftTimerId test_timer_id = 0;

  // For confirming test
  uint16_t actual_cell = 0;
  uint16_t device_cell = 0;

  // Reset discharge bitset
  test_afe_storage.discharge_bitset[balance_device] = AFE_DEFAULT_DISCHARGE_BITSET;
  // Only cell 2 should be balanced
  test_afe_storage.cell_voltages[0] = 1000;
  test_afe_storage.cell_voltages[1] = 1025;
  test_afe_storage.cell_voltages[2] = 1030;
  test_afe_storage.cell_voltages[3] = 1010;
  
  // Calculate device_cell in same way as ltc_afe_impl_toggle_cell_discharge would 
  // to check that bitset is as expected for the given cell
  actual_cell = test_afe_storage.discharge_cell_lookup[balance_cell];
  device_cell = actual_cell % LTC_AFE_MAX_CELLS_PER_DEVICE;
  passive_balance(&test_afe_storage);
  TEST_ASSERT_EQUAL(test_afe_storage.discharge_bitset[balance_device], AFE_DEFAULT_DISCHARGE_BITSET | (1 << device_cell)); 

  // First cell in range
  balance_cell = 0;

  // Reset discharge bitset
  test_afe_storage.discharge_bitset[balance_device] = AFE_DEFAULT_DISCHARGE_BITSET;
  // Only cell 0 should be balanced
  test_afe_storage.cell_voltages[0] = 1040;
  test_afe_storage.cell_voltages[1] = 1025;
  test_afe_storage.cell_voltages[2] = 1010;
  test_afe_storage.cell_voltages[3] = 1010;
  
  // Calculate device_cell in same way as ltc_afe_impl_toggle_cell_discharge would 
  // to check that bitset is as expected for the given cell
  actual_cell = test_afe_storage.discharge_cell_lookup[balance_cell];
  device_cell = actual_cell % LTC_AFE_MAX_CELLS_PER_DEVICE;
  passive_balance(&test_afe_storage);
  TEST_ASSERT_EQUAL(test_afe_storage.discharge_bitset[balance_device], AFE_DEFAULT_DISCHARGE_BITSET | (1 << device_cell)); 
  
  // Last cell in range 
  balance_cell = LTC_AFE_MAX_CELLS_PER_DEVICE * LTC_AFE_MAX_DEVICES - 1;
  balance_device = LTC_AFE_MAX_DEVICES - 1; 

  // Reset discharge bitset
  test_afe_storage.discharge_bitset[balance_device] = AFE_DEFAULT_DISCHARGE_BITSET;
  // Only last cell should be balanced
  test_afe_storage.cell_voltages[0] = 1000;
  test_afe_storage.cell_voltages[1] = 1025;
  test_afe_storage.cell_voltages[2] = 1030;
  test_afe_storage.cell_voltages[balance_cell] = 1040;
  
  // Calculate device_cell in same way as ltc_afe_impl_toggle_cell_discharge would 
  // to check that bitset is as expected for the given cell
  actual_cell = test_afe_storage.discharge_cell_lookup[balance_cell];
  device_cell = actual_cell % LTC_AFE_MAX_CELLS_PER_DEVICE;
  passive_balance(&test_afe_storage);
  TEST_ASSERT_EQUAL(test_afe_storage.discharge_bitset[balance_device], AFE_DEFAULT_DISCHARGE_BITSET | (1 << device_cell)); 
} 

// Balance values within allowed range, ensure they don't change
void test_balance_range(void) {
  LOG_DEBUG("Testing balancing around edge of range\n");
  // Same general procedures as above

  // Fill cells
  for(uint8_t i = NUM_TEST_VOLTAGES; i < LTC_AFE_MAX_CELLS; i++) {
    test_afe_storage.cell_voltages[i] = 1010;
  }
  
  // Storage variables
  uint8_t balance_cell = 0;
  uint8_t balance_device = 0;
  uint16_t actual_cell = 0;
  uint16_t device_cell = 0;

  // Remove this after rewriting passive_balance.c
  // If this is still here when I push I can't read
  SoftTimerId test_timer_id = 0;

  // Reset discharge bitset
  test_afe_storage.discharge_bitset[balance_device] = AFE_DEFAULT_DISCHARGE_BITSET;
  // No cell should be balanced
  test_afe_storage.cell_voltages[0] = 1000;
  test_afe_storage.cell_voltages[1] = 1024;
  test_afe_storage.cell_voltages[2] = 1000 + PASSIVE_BALANCE_MIN_VOLTAGE_DIFF_MV - 1;
  test_afe_storage.cell_voltages[3] = 1010;
  
  // Bitset should be unchanged
  
  passive_balance(&test_afe_storage);
  TEST_ASSERT_EQUAL(test_afe_storage.discharge_bitset[balance_device], AFE_DEFAULT_DISCHARGE_BITSET); 

  // Cell 2 should now be balanced 
  balance_cell = 2;
  test_afe_storage.cell_voltages[balance_cell] = 1000 + PASSIVE_BALANCE_MIN_VOLTAGE_DIFF_MV;

  actual_cell = test_afe_storage.discharge_cell_lookup[balance_cell];
  device_cell = actual_cell % LTC_AFE_MAX_CELLS_PER_DEVICE;
  passive_balance(&test_afe_storage);
  TEST_ASSERT_EQUAL(test_afe_storage.discharge_bitset[balance_device], AFE_DEFAULT_DISCHARGE_BITSET | (1 << device_cell)); 
}

