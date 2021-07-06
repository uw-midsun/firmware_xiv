#pragma once
// Fan control for MCI.  Currently hardcoded to turn the fan on to 100%
// Requires GPIO to be initialized
#include <inttypes.h>

#include "status.h"

// General pin definitions:
#define MCI_FAN_EN_ADDR \
  { GPIO_PORT_A, 0 }

// Possibly unused
#define MCI_FAN_PWM_ADDR \
  { GPIO_PORT_A, 1 }

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

// Fault types
typedef enum {
  MCI_FAN_CONTROL_DISCHARGE_OVERTEMP = 0,
  MCI_FAN_CONTROL_PRECHARGE_OVERTEMP,
  MCI_FAN_CONTROL_Q1_OVERTEMP,
  MCI_FAN_CONTROL_Q3_OVERTEMP,
  NUM_MCI_FAN_CONTROL_FAULTS,
} MciFanControlFault;

// Called when a fault occurs/clears.
typedef void (*MciFanControlFaultCallback)(uint8_t fault_bitset, void *context);

typedef struct {
  MciFanControlFaultCallback fault_cb;
  void *fault_context;
} MciFanControlSettings;

// Initialize fan control and turn on the fan.
StatusCode mci_fan_control_init(MciFanControlSettings *settings);
