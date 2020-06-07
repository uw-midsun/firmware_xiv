#pragma once
// Driver for LTC<CHANGED> AFE chip
// Assumes that:
// - a <CHANGED> channel analog MUX is attached to the GPIO outputs
// - <CHANGED> are used as AUX channel select outputs
// - <CHANGED> is used as a thermistor input
// Requires GPIO, Interrupts, Soft Timers, and Event Queue to be initialized
//
// Note that all units are in 100uV.
//
// This module supports AFEs with fewer than 12 cells using the |cell/aux_bitset|.
// Note that due to the long conversion delays required, we use an FSM to return control to the
// application.
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "spi.h"
#include "status.h"


// This is an arbitrary limitation, can be increased/decreased if needed
#define LTC_AFE_MAX_DEVICES 5
// This is a device limitation
#define LTC_AFE_MAX_CELLS_PER_DEVICE 12
#define LTC_AFE_MAX_CELLS (LTC_AFE_MAX_DEVICES * LTC_AFE_MAX_CELLS_PER_DEVICE)

#if defined(__GNUC__)
#define _PACKED __attribute__((packed))
#else
#define _PACKED
#endif

// Function to run when a conversion is complete
typedef void (*LtcAfeResultCallback)(uint16_t *result_arr, size_t len, void *context);

// select the ADC mode (trade-off between speed or minimizing noise)
// see p.50 for conversion times and p.23 for noise
typedef enum {
  LTC_AFE_ADC_MODE_27KHZ = 0,
  LTC_AFE_ADC_MODE_7KHZ,
  LTC_AFE_ADC_MODE_26HZ,
  LTC_AFE_ADC_MODE_14KHZ,
  LTC_AFE_ADC_MODE_3KHZ,
  LTC_AFE_ADC_MODE_2KHZ,
  NUM_LTC_AFE_ADC_MODES
} LtcAfeAdcMode;



typedef struct LtcAfeBitset {
  uint16_t cell_bitset;
  uint16_t aux_bitset;
} LtcAfeBitset;

typedef struct LtcAfeEventList {
  EventId trigger_cell_conv_event;
  EventId cell_conv_complete_event;
  EventId trigger_aux_conv_event;
  EventId aux_conv_complete_event;
  EventId callback_run_event;
  EventId fault_event;
} LtcAfeEventList;

typedef struct LtcAfeSettings {
  GpioAddress cs;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;

  const SpiPort spi_port;
  uint32_t spi_baudrate;

  LtcAfeAdcMode adc_mode;

  uint16_t cell_bitset[LTC_AFE_MAX_DEVICES];
  uint16_t aux_bitset[LTC_AFE_MAX_DEVICES];

  size_t num_devices;
  size_t num_cells;

  LtcAfeEventList ltc_events;
  LtcAfeResultCallback cell_result_cb;
  LtcAfeResultCallback aux_result_cb;
  void *result_context;
} LtcAfeSettings;


typedef struct LtcAfeStorage {
  Fsm fsm;

  // Only used for storage in the FSM so we store data for the correct cells
  uint16_t aux_index;
  uint16_t retry_count;

  uint16_t cell_voltages[LTC_AFE_MAX_CELLS];
  uint16_t aux_voltages[LTC_AFE_MAX_CELLS];

  uint16_t discharge_bitset[LTC_AFE_MAX_DEVICES];

  uint16_t cell_result_lookup[LTC_AFE_MAX_CELLS];
  uint16_t aux_result_lookup[LTC_AFE_MAX_CELLS];
  uint16_t discharge_cell_lookup[LTC_AFE_MAX_CELLS];

  LtcAfeSettings settings;
} LtcAfeStorage;

// Initialize the LTC<CHANGED>.
// |settings.cell_bitset| and |settings.aux_bitset| should be an array of bitsets where bits 0 to 11
// represent whether we should monitor the cell input for the given device.
// |settings.cell_result_cb| and |settings.aux_result_cb| will be called when the corresponding
// conversion is completed.
StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *settings);

StatusCode ltc_afe_set_result_cbs(LtcAfeStorage *afe, LtcAfeResultCallback cell_result_cb,
                                  LtcAfeResultCallback aux_result_cb, void *context);

// Raises trigger conversion events. These events must be processed.
StatusCode ltc_afe_request_cell_conversion(LtcAfeStorage *afe);
StatusCode ltc_afe_request_aux_conversion(LtcAfeStorage *afe);

// Process events
bool ltc_afe_process_event(LtcAfeStorage *afe, const Event *e);

// Mark cell for discharging (takes effect after config is re-written)
// |cell| should be [0, settings.max_total_cells)
StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge);
