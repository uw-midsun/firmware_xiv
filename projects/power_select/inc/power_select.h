#pragma once

// Measures values from power selection to send over CAN and handles faults.
// Requires gpio, interrupts, event queue, CAN, ADC (in ADC_MODE_SINGLE), and soft timers to be
// initialized.

#include <inttypes.h>
#include "adc.h"
#include "can_pack.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"

#include "power_select_defs.h"
#include "power_select_events.h"
#include "power_select_thermistor.h"

#define POWER_SELECT_MEASUREMENT_INTERVAL_MS 1000
#define POWER_SELECT_MEASUREMENT_INTERVAL_US ((POWER_SELECT_MEASUREMENT_INTERVAL_MS)*1000)

typedef enum {
  POWER_SELECT_AUX = 0,
  POWER_SELECT_DCDC,
  POWER_SELECT_PWR_SUP,
  NUM_POWER_SELECT_MEASUREMENT_TYPES,
} PowerSelectMeasurement;

#define NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS (NUM_POWER_SELECT_MEASUREMENT_TYPES)
#define NUM_POWER_SELECT_CURRENT_MEASUREMENTS (NUM_POWER_SELECT_MEASUREMENT_TYPES)
#define NUM_POWER_SELECT_TEMP_MEASUREMENTS \
  (NUM_POWER_SELECT_MEASUREMENT_TYPES - 1)  // no temp measurement for power supply

#define NUM_POWER_SELECT_MEASUREMENTS                                              \
  (NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS + NUM_POWER_SELECT_CURRENT_MEASUREMENTS + \
   NUM_POWER_SELECT_TEMP_MEASUREMENTS)

extern const GpioAddress g_power_select_cell_pin;

extern const GpioAddress g_power_select_voltage_pins[NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS];

extern const GpioAddress g_power_select_current_pins[NUM_POWER_SELECT_CURRENT_MEASUREMENTS];

extern const GpioAddress g_power_select_temp_pins[NUM_POWER_SELECT_TEMP_MEASUREMENTS];

extern const GpioAddress g_power_select_valid_pins[NUM_POWER_SELECT_VALID_PINS];

extern const uint16_t g_power_select_max_voltages[NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS];

extern const uint16_t g_power_select_max_currents[NUM_POWER_SELECT_CURRENT_MEASUREMENTS];

typedef struct {
  uint16_t voltages[NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS];
  uint16_t currents[NUM_POWER_SELECT_CURRENT_MEASUREMENTS];
  int32_t temps[NUM_POWER_SELECT_TEMP_MEASUREMENTS];
  uint16_t fault_bitset;
  uint16_t valid_bitset;
  uint16_t warning_bitset;
  SoftTimerId timer_id;
  bool measurement_in_progress;
  uint32_t interval_us;
} PowerSelectStorage;

// Initialize power selection
StatusCode power_select_init(void);

// Start periodically measuring sense values at the interval passed in
StatusCode power_select_start(uint32_t interval_us);

// Stop measuring sense values
bool power_select_stop(void);

// Return the fault bitset
uint16_t power_select_get_fault_bitset(void);

// Return the valid bitset
uint16_t power_select_get_valid_bitset(void);

// Return the storage (mainly for testing)
PowerSelectStorage power_select_get_storage(void);
