#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "crc15.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "ltc6811.h"
#include "ltc_afe.h"
#include "misc.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_LTC_AFE_NUM_DEVICES 1
#define TEST_LTC_AFE_NUM_CELLS 12

#define TEST_LTC_AFE_INPUT_BITSET_FULL 0xFFF
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

typedef enum {
  TEST_LTC_AFE_TRIGGER_CELL_CONV_EVENT = 0,
  TEST_LTC_AFE_CELL_CONV_COMPLETE_EVENT,
  TEST_LTC_AFE_TRIGGER_AUX_CONV_EVENT,
  TEST_LTC_AFE_AUX_CONV_COMPLETE_EVENT,
  TEST_LTC_AFE_CALLBACK_RUN_EVENT,
  TEST_LTC_AFE_FAULT_EVENT,
  NUM_TEST_LTC_EVENTS,
} TestLtcAfeEvents;

static LtcAfeStorage s_afe;
static uint16_t s_result_arr[TEST_LTC_AFE_NUM_CELLS];

StatusCode TEST_MOCK(spi_exchange)(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                                   size_t rx_len) {
  // Return a voltage of 0x6969 on voltage read
  TEST_ASSERT_EQUAL(spi, TEST_LTC_AFE_SPI_PORT);
  LtcAfeVoltageRegisterGroup registers[TEST_LTC_AFE_NUM_DEVICES] = {
    { .reg = { .voltages = { 0x6969, 0x420, 0x1337 } } }
  };

  for (int i = 0; i < TEST_LTC_AFE_NUM_DEVICES; i++) {
    registers[i].pec =
        SWAP_UINT16(crc15_calculate(registers[i].reg.values, sizeof(registers[i].reg)));
  }
  // Don't handle config packets
  if (tx_len != 4) return STATUS_CODE_OK;
  uint16_t cmd = (tx_data[0] << 8) | tx_data[1];
  switch (cmd) {
    case LTC6811_RDCVA_RESERVED:
    case LTC6811_RDCVB_RESERVED:
    case LTC6811_RDCVC_RESERVED:
    case LTC6811_RDCVD_RESERVED:
      TEST_ASSERT_NOT_NULL(rx_data);
      TEST_ASSERT_EQUAL(sizeof(registers), rx_len);
      memcpy(rx_data, registers, sizeof(registers));
      break;
    case LTC6811_RDAUXA_RESERVED:
    case LTC6811_RDAUXB_RESERVED:
      TEST_ASSERT_NOT_NULL(rx_data);
      TEST_ASSERT_EQUAL(sizeof(registers), rx_len);
      memcpy(rx_data, registers, sizeof(registers));
      break;
      break;
    case LTC6811_WRCOMM_RESERVED:
    case LTC6811_RDCOMM_RESERVED:
    case LTC6811_STCOMM_RESERVED:
    case LTC6811_RDCFG_RESERVED:
    case LTC6811_RDSTATA_RESERVED:
    case LTC6811_RDSTATB_RESERVED:
    default:
      TEST_ASSERT_NULL(rx_data);
      break;
  }
  return STATUS_CODE_OK;
}

static void prv_conv_cb(uint16_t *result_arr, size_t len, void *context) {
  TEST_ASSERT_EQUAL(TEST_LTC_AFE_NUM_CELLS, len);
  memcpy(s_result_arr, result_arr, sizeof(s_result_arr));
}

static void prv_wait_conv(void) {
  Event e = { 0 };
  do {
    MS_TEST_HELPER_AWAIT_EVENT(e);
    TEST_ASSERT_NOT_EQUAL(TEST_LTC_AFE_FAULT_EVENT, e.id);
    TEST_ASSERT_TRUE(ltc_afe_process_event(&s_afe, &e));
  } while (e.id != TEST_LTC_AFE_CALLBACK_RUN_EVENT);
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  LtcAfeSettings afe_settings = {
    .mosi = TEST_LTC_AFE_SPI_MOSI,
    .miso = TEST_LTC_AFE_SPI_MISO,
    .sclk = TEST_LTC_AFE_SPI_SCLK,
    .cs = TEST_LTC_AFE_SPI_CS,

    .spi_port = TEST_LTC_AFE_SPI_PORT,
    .spi_baudrate = TEST_LTC_AFE_SPI_BAUDRATE,
    .adc_mode = TEST_LTC_AFE_ADC_MODE,

    // Because these need to be the max size, should do initialize this way
    .cell_bitset = { 0 },
    .aux_bitset = { 0 },

    .num_devices = TEST_LTC_AFE_NUM_DEVICES,
    .num_cells = TEST_LTC_AFE_NUM_CELLS,

    .ltc_events = { .trigger_cell_conv_event = TEST_LTC_AFE_TRIGGER_CELL_CONV_EVENT,
                    .cell_conv_complete_event = TEST_LTC_AFE_CELL_CONV_COMPLETE_EVENT,
                    .trigger_aux_conv_event = TEST_LTC_AFE_TRIGGER_AUX_CONV_EVENT,
                    .aux_conv_complete_event = TEST_LTC_AFE_AUX_CONV_COMPLETE_EVENT,
                    .callback_run_event = TEST_LTC_AFE_CALLBACK_RUN_EVENT,
                    .fault_event = TEST_LTC_AFE_FAULT_EVENT },

    .cell_result_cb = prv_conv_cb,
    .aux_result_cb = prv_conv_cb,
    .result_context = NULL,
  };

  for (int i = 0; i < TEST_LTC_AFE_NUM_DEVICES; i++) {
    afe_settings.cell_bitset[i] = TEST_LTC_AFE_INPUT_BITSET_FULL;
    afe_settings.aux_bitset[i] = TEST_LTC_AFE_INPUT_BITSET_FULL;
  }

  ltc_afe_init(&s_afe, &afe_settings);
}

void teardown_test(void) {}

void test_ltc_afe_cell_conversion_initiated(void) {
  TEST_ASSERT_OK(ltc_afe_request_cell_conversion(&s_afe));
  prv_wait_conv();

  // Expect to read some value from cells
  for (int i = 0; i < TEST_LTC_AFE_NUM_CELLS; ++i) {
    TEST_ASSERT_NOT_EQUAL(0xFFFF, s_result_arr[i]);
    TEST_ASSERT_NOT_EQUAL(0x0000, s_result_arr[i]);
  }
}

void test_ltc_afe_aux_conversion_initiated(void) {
  TEST_ASSERT_OK(ltc_afe_request_aux_conversion(&s_afe));
  prv_wait_conv();

  // Expect to read some value from cells
  for (int i = 0; i < TEST_LTC_AFE_NUM_CELLS; ++i) {
    TEST_ASSERT_NOT_EQUAL(0xFFFF, s_result_arr[i]);
    TEST_ASSERT_NOT_EQUAL(0x0000, s_result_arr[i]);
  }
}

void test_ltc_afe_toggle_discharge_cells_valid_range(void) {
  uint16_t valid_cell = 0;
  StatusCode status = ltc_afe_toggle_cell_discharge(&s_afe, valid_cell, true);

  TEST_ASSERT_OK(status);
}

void test_ltc_afe_toggle_discharge_cells_invalid_range(void) {
  uint16_t invalid_cell = TEST_LTC_AFE_NUM_CELLS;
  StatusCode status = ltc_afe_toggle_cell_discharge(&s_afe, invalid_cell, true);

  TEST_ASSERT_NOT_OK(status);
}

void test_ltc_afe_toggle_discharge_cells_mapping(void) {
  // Modules we wish to enable
  uint16_t enable_modules[] = { 1u, 2u, 3u, 5u, 8u };

  StatusCode status = NUM_STATUS_CODES;
  // Ensure that all modules are set to off
  for (uint16_t cell = 0u; cell < TEST_LTC_AFE_NUM_CELLS; ++cell) {
    status = ltc_afe_toggle_cell_discharge(&s_afe, cell, false);
    TEST_ASSERT_OK(status);
  }

  // Now turn on bleed resistors for these specific modules
  for (size_t i = 0u; i < SIZEOF_ARRAY(enable_modules); ++i) {
    status = ltc_afe_toggle_cell_discharge(&s_afe, enable_modules[i], true);
    TEST_ASSERT_OK(status);
  }

  // Check the discharge mapping
  for (size_t i = 0u; i < SIZEOF_ARRAY(enable_modules); ++i) {
    // First lookup the physical index via the discharge_cell_lookup map
    uint16_t physical_index = s_afe.discharge_cell_lookup[enable_modules[i]];

    // Then separate it into the device and module
    uint16_t device = physical_index / LTC_AFE_MAX_CELLS_PER_DEVICE;
    uint16_t device_module = physical_index % LTC_AFE_MAX_CELLS_PER_DEVICE;

    TEST_ASSERT_EQUAL(1u, ((uint16_t)(s_afe.discharge_bitset[device] >> device_module)) & 1u);
  }
}
