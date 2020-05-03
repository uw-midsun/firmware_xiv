#pragma once
// Driver for LTC6804-1 AFE chip
// Assumes that:
// - a 16 channel analog MUX is attached to the GPIO outputs
// - GPIO2, GPIO3, GPIO4, GPIO5 are used as AUX channel select outputs
// - GPIO1 is used as a thermistor input
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
#include "fsm.h"
#include "gpio.h"
#include "plutus_cfg.h"
#include "spi.h"
#include "status.h"

#define LTC_AFE_MAX_CELLS_PER_DEVICE 12
#define LTC_AFE_MAX_TOTAL_CELLS (PLUTUS_CFG_AFE_DEVICES_IN_CHAIN * LTC_AFE_MAX_CELLS_PER_DEVICE)

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

typedef struct LtcAfeSettings {
  GpioAddress cs;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;

  const SpiPort spi_port;
  uint32_t spi_baudrate;

  LtcAfeAdcMode adc_mode;
  // Cell inputs to include in the measurement arrays
  // Should have |PLUTUS_CFG_AFE_TOTAL_CELLS| set bits.
  uint16_t cell_bitset[PLUTUS_CFG_AFE_DEVICES_IN_CHAIN];
  // Aux inputs to include in the measurement arrays
  // Should have |PLUTUS_CFG_AFE_TOTAL_CELLS| set bits.
  uint16_t aux_bitset[PLUTUS_CFG_AFE_DEVICES_IN_CHAIN];

  LtcAfeResultCallback cell_result_cb;
  LtcAfeResultCallback aux_result_cb;
  void *result_context;
} LtcAfeSettings;

typedef struct LtcAfeStorage {
  Fsm fsm;
  SpiPort spi_port;
  GpioAddress cs;
  LtcAfeAdcMode adc_mode;

  // Only used for storage in the FSM so we store data for the correct cells
  uint16_t aux_index;
  uint16_t retry_count;

  uint16_t cell_voltages[PLUTUS_CFG_AFE_TOTAL_CELLS];
  uint16_t aux_voltages[PLUTUS_CFG_AFE_TOTAL_CELLS];

  // Cell inputs to include in the measurement arrays
  uint16_t cell_bitset[PLUTUS_CFG_AFE_DEVICES_IN_CHAIN];
  // Lookup table for mapping AFE cell -> result index
  uint16_t cell_result_index[LTC_AFE_MAX_TOTAL_CELLS];

  // Aux inputs to include in the measurement arrays
  uint16_t aux_bitset[PLUTUS_CFG_AFE_DEVICES_IN_CHAIN];
  uint16_t aux_result_index[LTC_AFE_MAX_TOTAL_CELLS];

  // Discharge enabled - device-relative
  uint16_t discharge_bitset[PLUTUS_CFG_AFE_DEVICES_IN_CHAIN];
  // Lookup table for mapping cell -> actual device-relative AFE cell
  // discharge_cell_lookup[logical cell index] = physical cell index;
  // TODO(ELEC-447): Handle unused cell inputs during balancing
  uint16_t discharge_cell_lookup[LTC_AFE_MAX_TOTAL_CELLS];

  LtcAfeResultCallback cell_result_cb;
  LtcAfeResultCallback aux_result_cb;
  void *result_context;
} LtcAfeStorage;

// Initialize the LTC6804.
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

// Process PLUTUS_EVENT_AFE_* events
bool ltc_afe_process_event(LtcAfeStorage *afe, const Event *e);

// Mark cell for discharging (takes effect after config is re-written)
// |cell| should be [0, PLUTUS_CFG_AFE_TOTAL_CELLS)
StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge);
