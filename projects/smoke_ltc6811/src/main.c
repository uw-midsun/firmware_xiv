#include <string.h>

#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "ltc_afe.h"
#include "ltc_chip_config.h"
#include "smoke_ltc_events.h"
#include "soft_timer.h"
#include "status.h"

// smoke test settings
#define SMOKE_LTC_AFE_NUM_DEVICES 1
#define SMOKE_LTC_AFE_NUM_CELLS 12
#define SMOKE_LTC_AFE_INPUT_BITSET_FULL 0xFFF

// To disable samples set value to 0
#define SMOKE_LTC_AFE_NUM_VOLTAGE_SAMPLES 1
#define SMOKE_LTC_AFE_NUM_TEMP_SAMPLES 100

// To disable logging for an event, set the event name to NULL
static const char *s_event_names[NUM_SMOKE_LTC_EVENTS] = {
  [SMOKE_LTC_AFE_TRIGGER_CELL_CONV_EVENT] = "CELL CONVERSION TRIGGERED",
  [SMOKE_LTC_AFE_CELL_CONV_COMPLETE_EVENT] = "CELL CONVERSION COMPLETE",
  [SMOKE_LTC_AFE_TRIGGER_AUX_CONV_EVENT] = "AUX CONVERSION TRIGGERED",
  [SMOKE_LTC_AFE_AUX_CONV_COMPLETE_EVENT] = "AUX CONVERSION COMPLETE",
  [SMOKE_LTC_AFE_CALLBACK_RUN_EVENT] = "CONVERSION CALLBACK RUN",
  [SMOKE_LTC_AFE_FAULT_EVENT] = "FAULT OCCURED"
};

// data storage
typedef struct LtcAfeReadingBound {
  uint16_t min;
  uint16_t max;
} LtcAfeReadingBound;

static LtcAfeStorage s_afe;
static uint16_t s_result_arr[SMOKE_LTC_AFE_NUM_CELLS] = { 0 };
static size_t s_num_samples = 0;
LtcAfeReadingBound s_sample_bounds[SMOKE_LTC_AFE_NUM_CELLS] = { 0 };

static void prv_reset_sample_bounds(void) {
  for (int i = 0; i < SMOKE_LTC_AFE_NUM_CELLS; i++) {
    s_sample_bounds[i].min = UINT16_MAX;
    s_sample_bounds[i].max = 0;
  }
}

static StatusCode prv_extract_and_dump_readings(uint16_t *result_arr, size_t len,
                                                size_t max_samples) {
  if (len != sizeof(s_result_arr)) {
    LOG_WARN("Expected reading length to be %zu but it was %zu\n", sizeof(s_result_arr), len);
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(s_result_arr, result_arr, len);

  if (s_num_samples == 0) {
    LOG_DEBUG("INITIAL READINGS:");
  } else if (s_num_samples == max_samples - 1) {
    LOG_DEBUG("READING STATS:");
  }

  for (size_t cell = 0; cell < SMOKE_LTC_AFE_NUM_CELLS; ++cell) {
    s_sample_bounds[cell].min = MIN(s_result_arr[cell], s_sample_bounds[cell].min);
    s_sample_bounds[cell].max = MAX(s_result_arr[cell], s_sample_bounds[cell].max);

    if (s_num_samples == 0) {
      LOG_DEBUG("CELL#%zu = %d\n", cell, s_result_arr[cell]);
    } else if (s_num_samples == max_samples - 1) {
      uint16_t delta = s_sample_bounds[cell].max - s_sample_bounds[cell].min;
      LOG_DEBUG("CELL#%zu DELTA %d (MIN=%d, MAX=%d)\n", cell, delta, s_sample_bounds[cell].min,
                s_sample_bounds[cell].max);
    }
  }

  s_num_samples++;
  if (s_num_samples >= max_samples) {
    prv_reset_sample_bounds();
    s_num_samples = 0;
  }
  return STATUS_CODE_OK;
}

static void prv_dump_voltages(uint16_t *result_arr, size_t len, void *context) {
#if SMOKE_LTC_AFE_NUM_VOLTAGE_SAMPLES > 0
  if (!status_ok(prv_extract_and_dump_readings(result_arr, len, SMOKE_LTC_AFE_NUM_VOLTAGE_SAMPLES)))
    return;
#endif

  if (s_num_samples != 0) {
    if (!status_ok(ltc_afe_request_cell_conversion(&s_afe))) return;
  } else {
    LOG_DEBUG("==END OF VOLTAGE SAMPLES==");
    LOG_DEBUG("==START OF TEMPERATURE SAMPLES==");
    if (!status_ok(ltc_afe_request_aux_conversion(&s_afe))) return;
  }
}

static void prv_dump_temps(uint16_t *result_arr, size_t len, void *context) {
#if SMOKE_LTC_AFE_NUM_TEMP_SAMPLES > 0
  if (!status_ok(prv_extract_and_dump_readings(result_arr, len, SMOKE_LTC_AFE_NUM_TEMP_SAMPLES)))
    return;
#endif

  if (s_num_samples != 0) {
    if (!status_ok(ltc_afe_request_aux_conversion(&s_afe))) return;
  } else {
    LOG_DEBUG("==END OF TEMPERATURE SAMPLES==");
    // TODO(SOFT-222): Could test cell discharge here as well ...
  }
}

static StatusCode prv_ltc_init(void) {
  status_ok_or_return(gpio_init());
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  LtcAfeSettings afe_settings = {
    .mosi = SMOKE_LTC_AFE_SPI_MOSI,
    .miso = SMOKE_LTC_AFE_SPI_MISO,
    .sclk = SMOKE_LTC_AFE_SPI_SCLK,
    .cs = SMOKE_LTC_AFE_SPI_CS,

    .spi_port = SMOKE_LTC_AFE_SPI_PORT,
    .spi_baudrate = SMOKE_LTC_AFE_SPI_BAUDRATE,
    .adc_mode = SMOKE_LTC_AFE_ADC_MODE,

    // Because these need to be the max size, should do initialize this way
    .cell_bitset = { 0 },
    .aux_bitset = { 0 },

    .num_devices = SMOKE_LTC_AFE_NUM_DEVICES,
    .num_cells = SMOKE_LTC_AFE_NUM_CELLS,

    .ltc_events = { .trigger_cell_conv_event = SMOKE_LTC_AFE_TRIGGER_CELL_CONV_EVENT,
                    .cell_conv_complete_event = SMOKE_LTC_AFE_CELL_CONV_COMPLETE_EVENT,
                    .trigger_aux_conv_event = SMOKE_LTC_AFE_TRIGGER_AUX_CONV_EVENT,
                    .aux_conv_complete_event = SMOKE_LTC_AFE_AUX_CONV_COMPLETE_EVENT,
                    .callback_run_event = SMOKE_LTC_AFE_CALLBACK_RUN_EVENT,
                    .fault_event = SMOKE_LTC_AFE_FAULT_EVENT },

    .cell_result_cb = prv_dump_voltages,
    .aux_result_cb = prv_dump_temps,
    .result_context = NULL,
  };

  for (int i = 0; i < SMOKE_LTC_AFE_NUM_DEVICES; i++) {
    afe_settings.cell_bitset[i] = SMOKE_LTC_AFE_INPUT_BITSET_FULL;
    afe_settings.aux_bitset[i] = SMOKE_LTC_AFE_INPUT_BITSET_FULL;
  }

  status_ok_or_return(ltc_afe_init(&s_afe, &afe_settings));

  return STATUS_CODE_OK;
}

static void prv_log_event(const Event *event) {
  if (event->id < NUM_SMOKE_LTC_EVENTS && s_event_names[event->id] != NULL) {
    LOG_DEBUG("LTC AFE PROCESSING RESULT: %s\n", s_event_names[event->id]);
  }
}

int main(void) {
  // Initialize ltc driver
  prv_ltc_init();

  // Initialize bound variable
  prv_reset_sample_bounds();

  LOG_DEBUG("==START OF VOLTAGE SAMPLES==");
  status_ok_or_return(ltc_afe_request_cell_conversion(&s_afe));

  Event e = { 0 };
  while (true) {
    while (status_ok(event_process(&e))) {
      ltc_afe_process_event(&s_afe, &e);
      prv_log_event(&e);
    }
  }
}
