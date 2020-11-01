#pragma once
// FW for power selection board.
// Requires gpio, interrupts, CAN, and soft timers to be initialized.
// Handles reading current, voltage, and temp values from the power select
// board and broadcasting the results over CAN
// TODO: work in fault handling, disabling the LTC into the driver (probably after powertrain
// mockup)

#include "adc.h"
#include "can_transmit.h"
#include "gpio.h"
#include "soft_timer.h"
#include "status.h"
#include "gpio_it.h"

#include "power_select_defs.h"

#define POWER_SELECT_MEASUREMENT_INTERVAL_MS 100
#define POWER_SELECT_MEASUREMENT_INTERVAL_US (POWER_SELECT_MEASUREMENT_INTERVAL_MS * 1000)

typedef enum {
  POWER_SELECT_AUX = 0,
  POWER_SELECT_DCDC,
  POWER_SELECT_PWR_SUP,
  NUM_POWER_SELECT_MEASUREMENTS,
} PowerSelectMeasurement;

#define NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS (NUM_POWER_SELECT_MEASUREMENTS)
#define NUM_POWER_SELECT_CURRENT_MEASUREMENTS (NUM_POWER_SELECT_MEASUREMENTS)
#define NUM_POWER_SELECT_TEMP_MEASUREMENTS (NUM_POWER_SELECT_MEASUREMENTS - 1)

// Storage for previous measurements
typedef struct {
  uint16_t voltages[NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS];
  uint16_t currents[NUM_POWER_SELECT_CURRENT_MEASUREMENTS];
  uint16_t temps[NUM_POWER_SELECT_TEMP_MEASUREMENTS];  // no power supply temp measurements
  bool fault_status;
} PowerSelectStorage;

// Initialize power selection
StatusCode power_select_init(void);

// Start periodically measuring sense values
StatusCode power_select_start(void);