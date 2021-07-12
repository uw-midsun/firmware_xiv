#pragma once
// Fan control for MCI based on thermistor status pins.
// Requires GPIO, interrupts, and gpio interrupts to be initialized.
#include <inttypes.h>

#include "status.h"
#include "gpio.h"

// General pin definitions:
#define MCI_FAN_EN_ADDR \
  { GPIO_PORT_A, 0 }

// Connected to fan tachometers
#define MCI_FAN_1_SENSE_ADDR \
  { GPIO_PORT_A, 2 }
#define MCI_FAN_2_SENSE_ADDR \
  { GPIO_PORT_A, 3 }

// Thermistors
#define MCI_DISCHARGE_OVERTEMP_ADDR \
  { GPIO_PORT_A, 4 }
#define MCI_PRECHARGE_OVERTEMP_ADDR \
  { GPIO_PORT_A, 5 }
#define MCI_Q1_OVERTEMP_ADDR \
  { GPIO_PORT_A, 6 }
#define MCI_Q3_OVERTEMP_ADDR \
  { GPIO_PORT_A, 7 }

// Thermistor indices
typedef enum {
  MCI_THERM_DISCHARGE_OVERTEMP = 0,
  MCI_THERM_PRECHARGE_OVERTEMP,
  MCI_THERM_Q1_OVERTEMP,
  MCI_THERM_Q3_OVERTEMP,
  NUM_MCI_FAN_CONTROL_THERMS,
} MciFanControlTherm;

extern const GpioAddress g_therm_addrs[NUM_MCI_FAN_CONTROL_THERMS];

// Fan states
typedef enum {
  MCI_FAN_STATE_OFF = 0,
  MCI_FAN_STATE_ON,
  NUM_MCI_FAN_STATES,
} MciFanState;

// Called when a fault occurs/clears.
typedef void (*MciFanControlFaultCallback)(uint8_t fault_bitset, void *context);

typedef struct {
  MciFanControlFaultCallback fault_cb;
  void *fault_context;
} MciFanControlSettings;

typedef struct {
  MciFanControlFaultCallback fault_cb;
  void *fault_context;
  uint8_t fault_bitset;
} MciFanControlStorage;

// Initialize fan control and turn on the fan.
StatusCode mci_fan_control_init(MciFanControlSettings *settings);

StatusCode mci_fan_set_state(MciFanState state);
