// Brief test sequence for passive balancing module
#include "passive_balance.h"

#include "delay.h"
#include "interrupt.h"
#include "ms_test_helpers.h"
#include "ltc_afe.h"
#include "bms_events.h"
#include "stdio.h"
#include "string.h"

//temp
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

#include "log.h"

// Number of voltages to be tested out of all voltages
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
  LOG_DEBUG("got here\n");
  ltc_afe_init(&test_afe_storage, &test_afe_settings); 
  LOG_DEBUG("got here\n");
  interrupt_init();
  TEST_ASSERT_OK(passive_balance_init(&test_afe_storage));

}

void teardown_test(void) {}

// Balance a few values, ensure cell bitset is set properly.
void test_normal_operation(void) {
  // Doing all tests on first 4 cells of device 0, as max number of devices is is variable 
  // Fill rest with numbers from 1010 to 1020
  for(uint8_t i = NUM_TEST_VOLTAGES; i < LTC_AFE_MAX_CELLS; i++) {
    test_afe_storage.cell_voltages[i] = 1010;
  }
  
  // Stores cell and device that should be balanced
  uint8_t balance_cell = 2;
  uint8_t balance_device = 0;

  // Reset discharge bitset
  test_afe_storage.discharge_bitset[balance_device] = AFE_DEFAULT_DISCHARGE_BITSET;
  // Only cell 2 should be balanced
  test_afe_storage.cell_voltages[0] = 1000;
  test_afe_storage.cell_voltages[1] = 1025;
  test_afe_storage.cell_voltages[2] = 1030;
  test_afe_storage.cell_voltages[3] = 1010;
  
  LOG_DEBUG("value before:\n");
  LOG_DEBUG("%d\n", test_afe_storage.discharge_bitset[balance_device]);
  uint16_t actual_cell = test_afe_storage.discharge_cell_lookup[balance_cell];
  uint16_t device_cell = actual_cell % LTC_AFE_MAX_CELLS_PER_DEVICE;
  // Wait for 2 passive balance intervals 
  //delay_ms(PASSIVE_BALANCE_INTERVAL_MS * 2); 
  SoftTimerId test = 0;
  passive_balance(test, &test_afe_storage);
  LOG_DEBUG("value after:\n");
  LOG_DEBUG("%d\n", test_afe_storage.discharge_bitset[balance_device]);
  LOG_DEBUG("expected value: %d\n", AFE_DEFAULT_DISCHARGE_BITSET | (1 << device_cell));
  TEST_ASSERT_EQUAL(test_afe_storage.discharge_bitset[balance_device], AFE_DEFAULT_DISCHARGE_BITSET | (1 << device_cell)); 
  LOG_DEBUG("%d\n", test_afe_storage.discharge_bitset[balance_device]);

  /*
  // First cell
  balance_cell = 0;

  // Reset discharge bitset
  test_afe_storage.discharge_bitset[balance_device] = AFE_DEFAULT_DISCHARGE_BITSET;
  // Only cell 0 should be balanced
  test_afe_storage.cell_voltages[0] = 1040;
  test_afe_storage.cell_voltages[1] = 1025;
  test_afe_storage.cell_voltages[2] = 1010;
  test_afe_storage.cell_voltages[3] = 1010;
  
  // Wait for 2 passive balance intervals
  delay_ms(PASSIVE_BALANCE_INTERVAL_MS * 2); 

  TEST_ASSERT_EQUAL(test_afe_storage.discharge_bitset[balance_device], AFE_DEFAULT_DISCHARGE_BITSET | (1 << balance_cell)); 

  // Last cell
  balance_cell = LTC_AFE_MAX_CELLS_PER_DEVICE * LTC_AFE_MAX_DEVICES - 1;
  balance_device = LTC_AFE_MAX_DEVICES - 1; 

  // Reset discharge bitset
  test_afe_storage.discharge_bitset[balance_device] = AFE_DEFAULT_DISCHARGE_BITSET;
  // Only last cell should be balanced
  test_afe_storage.cell_voltages[0] = 1000;
  test_afe_storage.cell_voltages[1] = 1025;
  test_afe_storage.cell_voltages[2] = 1030;
  test_afe_storage.cell_voltages[balance_cell] = 1040;
  
  // Wait for 2 passive balance intervals
  delay_ms(PASSIVE_BALANCE_INTERVAL_MS * 2); 

  TEST_ASSERT_EQUAL(test_afe_storage.discharge_bitset[balance_device], AFE_DEFAULT_DISCHARGE_BITSET | (1 << balance_cell)); 
  */
} 

// Balance values, ensure they don't change
void test_within_balance_range(void) {
  // same tests as above but edge case is 24 no balance / 25 balance
}

