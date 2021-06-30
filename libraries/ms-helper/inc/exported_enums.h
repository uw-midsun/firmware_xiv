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
  EE_CONSOLE_FAULT_AREA_DRIVE_FSM = 0,
  EE_CONSOLE_FAULT_AREA_POWER_MAIN,
  EE_CONSOLE_FAULT_AREA_POWER_OFF,
  EE_CONSOLE_FAULT_AREA_POWER_AUX,
  EE_CONSOLE_FAULT_AREA_BPS_HEARTBEAT,
  NUM_EE_CONSOLE_FAULT_AREAS
} EEConsoleFaultArea;

typedef enum {
  EE_DRIVE_FSM_STEP_MCI_RELAY_STATE = 0,
  EE_DRIVE_FSM_STEP_PRECHARGE_TIMEOUT,
  EE_DRIVE_FSM_STEP_EBRAKE_STATE,
  EE_DRIVE_FSM_STEP_MCI_OUTPUT,
  NUM_EE_DRIVE_FSM_STEPS
} EEDriveFsmStep;

typedef enum {
  EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS = 0,
  EE_POWER_MAIN_SEQUENCE_TURN_ON_DRIVER_BMS,
  EE_POWER_MAIN_SEQUENCE_CONFIRM_BATTERY_STATUS,
  EE_POWER_MAIN_SEQUENCE_CLOSE_BATTERY_RELAYS,
  EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC,
  EE_POWER_MAIN_SEQUENCE_TURN_ON_EVERYTHING,
  NUM_EE_POWER_MAIN_SEQUENCES
} EEPowerMainSequence;

typedef enum {
  EE_POWER_OFF_SEQUENCE_DISCHARGE_PRECHARGE = 0,
  EE_POWER_OFF_SEQUENCE_TURN_OFF_EVERYTHING,
  EE_POWER_OFF_SEQUENCE_OPEN_BATTERY_RELAYS,
  NUM_EE_POWER_OFF_SEQUENCES
} EEPowerOffSequence;

typedef enum {
  EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS = 0,
  EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING,
  NUM_EE_POWER_AUX_SEQUENCES
} EEPowerAuxSequence;

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

typedef enum {
  EE_DRIVE_OUTPUT_OFF = 0,
  EE_DRIVE_OUTPUT_DRIVE,
  EE_DRIVE_OUTPUT_REVERSE,
  NUM_EE_DRIVE_OUTPUTS,
} EEDriveOutput;

typedef enum {
  EE_RELAY_ID_BATTERY = 0,
  EE_RELAY_ID_MOTOR_CONTROLLER,
  EE_RELAY_ID_SOLAR,
  NUM_EE_RELAY_IDS,
} EERelayId;

// Light type to be used with a SYSTEM_CAN_MESSAGE_LIGHTS_STATE message.
typedef enum EELightType {
  EE_LIGHT_TYPE_DRL = 0,
  EE_LIGHT_TYPE_BRAKES,
  EE_LIGHT_TYPE_STROBE,
  EE_LIGHT_TYPE_SIGNAL_RIGHT,
  EE_LIGHT_TYPE_SIGNAL_LEFT,
  EE_LIGHT_TYPE_SIGNAL_HAZARD,
  EE_LIGHT_TYPE_HIGH_BEAMS,
  EE_LIGHT_TYPE_LOW_BEAMS,
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

typedef enum {
  EE_RELAY_STATE_OPEN = 0,
  EE_RELAY_STATE_CLOSE,
  NUM_EE_RELAY_STATES,
} EERelayState;

// For battery heartbeat
typedef enum EEBatteryHeartbeatFaultSource {
  EE_BPS_FAULT_SOURCE_KILLSWITCH = 0,
  EE_BPS_FAULT_SOURCE_AFE_CELL,
  EE_BPS_FAULT_SOURCE_AFE_TEMP,
  EE_BPS_FAULT_SOURCE_AFE_FSM,
  EE_BPS_FAULT_SOURCE_RELAY,
  EE_BPS_FAULT_SOURCE_CURRENT_SENSE,
  EE_BPS_FAULT_SOURCE_ACK_TIMEOUT,
  NUM_EE_BPS_FAULT_SOURCES,
} EEBatteryHeartbeatFaultSource;

// Battery heartbeat bitset representing fault reason
typedef uint8_t EEBatteryHeartbeatState;
#define EE_BPS_STATE_OK 0x0
#define EE_BPS_STATE_FAULT_KILLSWITCH (1 << EE_BPS_FAULT_SOURCE_KILLSWITCH)
#define EE_BPS_STATE_FAULT_AFE_CELL (1 << EE_BPS_FAULT_SOURCE_AFE_CELL)
#define EE_BPS_STATE_FAULT_AFE_TEMP (1 << EE_BPS_FAULT_SOURCE_AFE_TEMP)
#define EE_BPS_STATE_FAULT_AFE_FSM (1 << EE_BPS_FAULT_SOURCE_AFE_FSM)
#define EE_BPS_STATE_FAULT_RELAY (1 << EE_BPS_FAULT_SOURCE_RELAY)
#define EE_BPS_STATE_FAULT_CURRENT_SENSE (1 << EE_BPS_FAULT_SOURCE_CURRENT_SENSE)
#define EE_BPS_STATE_FAULT_ACK_TIMEOUT (1 << EE_BPS_FAULT_SOURCE_ACK_TIMEOUT)

typedef enum {
  EE_DRIVE_STATE_DRIVE = 0,
  EE_DRIVE_STATE_NEUTRAL,
  EE_DRIVE_STATE_REVERSE,
  EE_DRIVE_STATE_PARKING,
  NUM_EE_DRIVE_STATES,
} EEDriveState;

typedef enum {
  EE_CRUISE_CONTROL_COMMAND_TOGGLE = 0,
  EE_CRUISE_CONTROL_COMMAND_INCREASE,
  EE_CRUISE_CONTROL_COMMAND_DECREASE,
  NUM_EE_CRUISE_CONTROL_COMMANDS
} EECruiseControl;

#define EE_PEDAL_VALUE_DENOMINATOR ((1 << 12))

typedef enum EEChargerFault {
  EE_CHARGER_FAULT_HARDWARE_FAILURE = 0,
  EE_CHARGER_FAULT_OVER_TEMP,
  EE_CHARGER_FAULT_WRONG_VOLTAGE,
  EE_CHARGER_FAULT_POLARITY_FAILURE,
  EE_CHARGER_FAULT_COMMUNICATION_TIMEOUT,
  EE_CHARGER_FAULT_CHARGER_OFF,
  NUM_EE_CHARGER_FAULTS,
} EEChargerFault;

typedef enum EESolarFault {
  // An MCP3427 is faulting too much, data is the solar DataPoint associated with the faulty MCP3427
  EE_SOLAR_FAULT_MCP3427 = 0,

  // An MPPT had an overcurrent, the least significant 4 bits of data is the index of the MPPT
  // that faulted, the most significant 4 bits is a 4-bit bitmask of which branches faulted.
  EE_SOLAR_FAULT_MPPT_OVERCURRENT,

  // An MPPT had an overvoltage or overtemperature, data is the index of the MPPT that faulted
  EE_SOLAR_FAULT_MPPT_OVERVOLTAGE,
  EE_SOLAR_FAULT_MPPT_OVERTEMPERATURE,

  // The current from the whole array is over the threshold. No data.
  EE_SOLAR_FAULT_OVERCURRENT,

  // The current from the whole array is negative, so we aren't charging. No data.
  EE_SOLAR_FAULT_NEGATIVE_CURRENT,

  // The sum of the sensed voltages is over the threshold. No data.
  EE_SOLAR_FAULT_OVERVOLTAGE,

  // The temperature of any array thermistor is over our threshold. Data is the index of the too-hot
  // thermistor.
  EE_SOLAR_FAULT_OVERTEMPERATURE,

  // The temperature of the fan is over the threshold. No data.
  EE_SOLAR_FAULT_FAN_OVERTEMPERATURE,

  // Fan failure detected. No data.
  EE_SOLAR_FAULT_FAN_FAIL,

  // Relay failure to open
  EE_SOLAR_RELAY_OPEN_ERROR,

  NUM_EE_SOLAR_FAULTS,
} EESolarFault;

typedef enum EESolarRelayOpenErrorReason {
  // The drv120 relay has signaled that overtemp/undervolt lockout conditions have been triggered
  EE_SOLAR_RELAY_ERROR_DRV120,
  // The drv120 relay has not opened or the current has exceeded
  EE_SOLAR_RELAY_ERROR_CURRENT_EXCEEDED_NOT_OPEN,
  // The drv120 relay's current has not been set
  EE_RELAY_ERROR_CURRENT_NEVER_SET,

  NUM_EE_SOLAR_RELAY_OPEN_ERROR_REASON
} EESolarRelayOpenErrorReason;
