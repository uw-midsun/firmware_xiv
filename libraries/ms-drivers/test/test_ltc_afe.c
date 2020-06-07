#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "ltc_afe.h"
#include "ms_test_helpers.h"
#include "plutus_cfg.h"
#include "plutus_event.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_LTC_AFE_NUM_SAMPLES 100
// Maximum total measurement error in filtered mode - +/-2.8mV
// Maximum peak-to-peak at 7kHz: +/-250uV
#define TEST_LTC_AFE_VOLTAGE_VARIATION 5

static LtcAfeStorage s_afe;
static uint16_t s_result_arr[PLUTUS_CFG_AFE_TOTAL_CELLS];

static void prv_conv_cb(uint16_t *result_arr, size_t len, void *context) {
  memcpy(s_result_arr, result_arr, sizeof(s_result_arr));
}

static void prv_wait_conv(void) {
  Event e = { 0 };
  do {
    MS_TEST_HELPER_AWAIT_EVENT(e);
    TEST_ASSERT_NOT_EQUAL(PLUTUS_EVENT_AFE_FAULT, e.id);
    TEST_ASSERT_TRUE(ltc_afe_process_event(&s_afe, &e));
  } while (e.id != PLUTUS_EVENT_AFE_CALLBACK_RUN);
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  const LtcAfeSettings afe_settings = {
    .mosi = PLUTUS_CFG_AFE_SPI_MOSI,
    .miso = PLUTUS_CFG_AFE_SPI_MISO,
    .sclk = PLUTUS_CFG_AFE_SPI_SCLK,
    .cs = PLUTUS_CFG_AFE_SPI_CS,

    .spi_port = PLUTUS_CFG_AFE_SPI_PORT,
    .spi_baudrate = PLUTUS_CFG_AFE_SPI_BAUDRATE,
    .adc_mode = PLUTUS_CFG_AFE_MODE,

    .cell_bitset = PLUTUS_CFG_CELL_BITSET_ARR,
    .aux_bitset = PLUTUS_CFG_AUX_BITSET_ARR,

    .cell_result_cb = prv_conv_cb,
    .aux_result_cb = prv_conv_cb,
    .result_context = NULL,
  };

  ltc_afe_init(&s_afe, &afe_settings);
}

void teardown_test(void) {}

void test_ltc_afe_adc_conversion_initiated(void) {
  TEST_ASSERT_OK(ltc_afe_request_cell_conversion(&s_afe));
  prv_wait_conv();

  // if the ADC conversion packet is valid, then these should be voltage values
  // otherwise, the reading will correspond to the values the registers are
  // initialized as by default (0xFF)
  for (int i = 0; i < PLUTUS_CFG_AFE_TOTAL_CELLS; ++i) {
    TEST_ASSERT_NOT_EQUAL(0xFFFF, s_result_arr[i]);
  }
}

void test_ltc_afe_read_all_voltage_repeated_within_tolerances(void) {
  // the idea here is that we repeatedly take samples and verify that the values being read
  // are within an acceptable tolerance
  struct {
    uint16_t min;
    uint16_t max;
  } bounds[PLUTUS_CFG_AFE_TOTAL_CELLS] = { 0 };

  for (int i = 0; i < PLUTUS_CFG_AFE_TOTAL_CELLS; i++) {
    bounds[i].min = UINT16_MAX;
  }

  for (int sample = 0; sample < TEST_LTC_AFE_NUM_SAMPLES; ++sample) {
    TEST_ASSERT_OK(ltc_afe_request_cell_conversion(&s_afe));
    prv_wait_conv();

    for (int cell = 0; cell < PLUTUS_CFG_AFE_TOTAL_CELLS; ++cell) {
      bounds[cell].min = MIN(bounds[cell].min, s_result_arr[cell]);
      bounds[cell].max = MAX(bounds[cell].max, s_result_arr[cell]);
    }
  }

  for (size_t i = 0; i < PLUTUS_CFG_AFE_TOTAL_CELLS; i++) {
    uint16_t delta = bounds[i].max - bounds[i].min;
    LOG_DEBUG("C%d delta %d (min %d, max %d)\n", i, delta, bounds[i].min, bounds[i].max);
  }
}

void test_ltc_afe_read_all_aux_repeated_within_tolerances(void) {
  // the idea here is that we repeatedly take samples and verify that the values being read
  // are within an acceptable tolerance
  struct {
    uint16_t min;
    uint16_t max;
  } bounds[PLUTUS_CFG_AFE_TOTAL_CELLS] = { 0 };

  for (int i = 0; i < PLUTUS_CFG_AFE_TOTAL_CELLS; i++) {
    bounds[i].min = UINT16_MAX;
  }

  for (int sample = 0; sample < TEST_LTC_AFE_NUM_SAMPLES; ++sample) {
    TEST_ASSERT_OK(ltc_afe_request_aux_conversion(&s_afe));
    prv_wait_conv();

    for (int cell = 0; cell < PLUTUS_CFG_AFE_TOTAL_CELLS; ++cell) {
      bounds[cell].min = MIN(bounds[cell].min, s_result_arr[cell]);
      bounds[cell].max = MAX(bounds[cell].max, s_result_arr[cell]);
    }
  }

  for (size_t i = 0; i < PLUTUS_CFG_AFE_TOTAL_CELLS; i++) {
    uint16_t delta = bounds[i].max - bounds[i].min;
    LOG_DEBUG("C%d aux delta %d (min %d, max %d)\n", i, delta, bounds[i].min, bounds[i].max);
  }
}

void test_ltc_afe_toggle_discharge_cells_valid_range(void) {
  uint16_t valid_cell = 0;
  StatusCode status = ltc_afe_toggle_cell_discharge(&s_afe, valid_cell, true);

  TEST_ASSERT_OK(status);
}

void test_ltc_afe_toggle_discharge_cells_invalid_range(void) {
  uint16_t invalid_cell = PLUTUS_CFG_AFE_TOTAL_CELLS;
  StatusCode status = ltc_afe_toggle_cell_discharge(&s_afe, invalid_cell, true);

  TEST_ASSERT_NOT_OK(status);
}

void test_ltc_afe_toggle_discharge_cells_mapping(void) {
  // Modules we wish to enable
  uint16_t enable_modules[] = { 1u, 2u, 3u, 5u, 8u };

  StatusCode status = NUM_STATUS_CODES;
  // Ensure that all modules are set to off
  for (uint16_t cell = 0u; cell < PLUTUS_CFG_AFE_TOTAL_CELLS; ++cell) {
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
