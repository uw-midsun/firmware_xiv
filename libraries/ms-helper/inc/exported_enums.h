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

// Front Power Distribution Output to be used with a SYSTEM_CAN_MESSAGE_FRONT_POWER message.
typedef enum {
  EE_FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY = 0,
  EE_FRONT_POWER_DISTRIBUTION_OUTPUT_STEERING,
  EE_FRONT_POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE,
  EE_FRONT_POWER_DISTRIBUTION_OUTPUT_PEDAL,
  EE_FRONT_POWER_DISTRIBUTION_OUTPUT_FRONT_HEADLIGHTS_LEFT,
  EE_FRONT_POWER_DISTRIBUTION_OUTPUT_STROBE,
  EE_FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_FANS
} EEFrontPowerDistributionOutput;

// Front Power Distribution Output State to be used with a SYSTEM_CAN_MESSAGE_FRONT_POWER message.
typedef enum {
  EE_FRONT_POWER_DISTRIBUTION_OUTPUT_STATE_OFF = 0,
  EE_FRONT_POWER_DISTRIBUTION_OUTPUT_STATE_ON,
  NUM_EE_FRONT_POWER_DISTRIBUTION_OUTPUT_STATES
} EEFrontPowerDistributionOutputState;

// Light type to be used with a SYSTEM_CAN_MESSAGE_LIGHTS_STATE message.
typedef enum EELightType {
  EE_LIGHT_TYPE_HIGH_BEAMS = 0,
  EE_LIGHT_TYPE_LOW_BEAMS,
  EE_LIGHT_TYPE_DRL,
  EE_LIGHT_TYPE_BRAKES,
  EE_LIGHT_TYPE_SIGNAL_RIGHT,
  EE_LIGHT_TYPE_SIGNAL_LEFT,
  EE_LIGHT_TYPE_SIGNAL_HAZARD,
  EE_LIGHT_TYPE_STROBE,
  NUM_EE_LIGHT_TYPES,
} EELightType;

// Light state to be used with a SYSTEM_CAN_MESSAGE_LIGHTS message.
typedef enum EELightState {
  EE_LIGHT_STATE_OFF = 0,  //
  EE_LIGHT_STATE_ON,       //
  NUM_EE_LIGHT_STATES,     //
} EELightState;

// Horn state, used with a SYSTEM_CAN_MESSAGE_HORN message.
typedef enum EEHornState {
  EE_HORN_STATE_OFF = 0,  //
  EE_HORN_STATE_ON,       //
  NUM_EE_HORN_STATES,     //
} EEHornState;

// Used with most _RELAY messages to request a relay state change.
typedef enum EERelayState {
  EE_RELAY_STATE_OPEN = 0,
  EE_RELAY_STATE_CLOSE,
  NUM_EE_RELAY_STATES,
} EERelayState;

