#pragma once

#include "can_msg_defs.h"

// This file stores enums which are exported between projects to allow both
// sides to use the same enums when sending and receiving CAN Messages over the
// primary network. To make things easier all enums in this file must follow a
// slightly modified naming convention.
//
// Example:
// typedef enum {
//   EE_<MY_CAN_MESSAGE_NAME>_<FIELD_NAME>_<VALUE> = 0,
//   // ...
//   NUM_EE_<MY_CAN_MESSAGE_NAME>_<FIELD_NAME>_<PLURAL>,
// } EE<MyCanMessageName><FieldName>

typedef enum {
  EE_CHARGER_SET_RELAY_STATE_OPEN = 0,
  EE_CHARGER_SET_RELAY_STATE_CLOSE,
  NUM_EE_CHARGER_SET_RELAY_STATES,
} EEChargerSetRelayState;

typedef enum {
  EE_CHARGER_CONN_STATE_DISCONNECTED = 0,
  EE_CHARGER_CONN_STATE_CONNECTED,
  NUM_EE_CHARGER_CONN_STATES,
} EEChargerConnState;

// Drive output
// Mech brake + throttle
#define EE_DRIVE_OUTPUT_DENOMINATOR (1 << 12)
// Arbitrary 5% minimum pressure before considering it as engaged
#define EE_DRIVE_OUTPUT_MECH_THRESHOLD (5 * (EE_DRIVE_OUTPUT_DENOMINATOR) / 100)

typedef enum {
  EE_FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY = 0,
  NUM_EE_FRONT_POWER_DISTRIBUTION_OUTPUTS,
} EEFrontPowerDistributionOutput;

typedef enum {
  EE_REAR_POWER_DISTRIBUTION_OUTPUT_BMS_CARRIER = 0,
  EE_REAR_POWER_DISTRIBUTION_OUTPUT_MCI,
  NUM_EE_REAR_POWER_DISTRIBUTION_OUTPUTS,
} EERearPowerDistributionOutput;

typedef enum {
  EE_RELAY_ID_BATTERY = 0,
  EE_RELAY_ID_MOTOR_CONTROLLER,
  EE_RELAY_ID_SOLAR,
  NUM_EE_RELAY_IDS,
} EERelayId;

