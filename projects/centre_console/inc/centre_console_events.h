#pragma once

typedef enum {
  CENTRE_CONSOLE_EVENT_CAN_RX = 0,
  CENTRE_CONSOLE_EVENT_CAN_TX,
  CENTRE_CONSOLE_EVENT_CAN_FAULT,
  NUM_CENTRE_CONSOLE_CAN_EVENTS  // 3
} CentreConsoleCanEvent;

typedef enum {
  CENTRE_CONSOLE_POWER_EVENT_OFF = NUM_CENTRE_CONSOLE_CAN_EVENTS,  // 3
  CENTRE_CONSOLE_POWER_EVENT_ON_MAIN,
  CENTRE_CONSOLE_POWER_EVENT_ON_AUX,
  CENTRE_CONSOLE_POWER_EVENT_CLEAR_FAULT,
  CENTRE_CONSOLE_POWER_EVENT_FAULT,
  NUM_CENTRE_CONSOLE_POWER_EVENTS  // 11
} CentreConsolePowerEvent;

typedef enum {
  POWER_AUX_SEQUENCE_EVENT_BEGIN = NUM_CENTRE_CONSOLE_POWER_EVENTS,  // 11
  POWER_AUX_SEQUENCE_EVENT_FAULT,
  POWER_AUX_SEQUENCE_EVENT_AUX_STATUS_OK,
  POWER_AUX_SEQUENCE_EVENT_TURNED_ON_EVERYTHING,
  POWER_AUX_SEQUENCE_EVENT_COMPLETE,
  NUM_POWER_AUX_SEQUENCE_EVENTS  // 16
} PowerAuxSequenceEvent;

typedef enum {
  POWER_OFF_SEQUENCE_EVENT_BEGIN = NUM_POWER_AUX_SEQUENCE_EVENTS,  // 16
  POWER_OFF_SEQUENCE_EVENT_FAULT,
  POWER_OFF_SEQUENCE_EVENT_DISCHARGE_COMPLETED,
  POWER_OFF_SEQUENCE_EVENT_TURNED_OFF_EVERYTHING,
  POWER_OFF_SEQUENCE_EVENT_BATTERY_RELAYS_OPENED,
  POWER_OFF_SEQUENCE_EVENT_COMPLETE,
  NUM_POWER_OFF_SEQUENCE_EVENTS  // 21
} PowerOffSequenceEvent;

typedef enum {
  POWER_MAIN_SEQUENCE_EVENT_BEGIN = NUM_POWER_OFF_SEQUENCE_EVENTS,  // 21
  POWER_MAIN_SEQUENCE_EVENT_FAULT,
  POWER_MAIN_SEQUENCE_EVENT_NO_OP,
  POWER_MAIN_SEQUENCE_EVENT_AUX_STATUS_OK,
  POWER_MAIN_SEQUENCE_EVENT_DRIVER_DISPLAY_BMS_ON,
  POWER_MAIN_SEQUENCE_EVENT_BATTERY_STATUS_OK,
  POWER_MAIN_SEQUENCE_EVENT_BATTERY_RELAYS_CLOSED,
  POWER_MAIN_SEQUENCE_EVENT_DC_DC_OK,
  POWER_MAIN_SEQUENCE_EVENT_TURNED_ON_EVERYTHING,
  POWER_MAIN_SEQUENCE_EVENT_COMPLETE,
  NUM_POWER_MAIN_SEQUENCE_EVENTS  // 31
} PowerMainSequenceEvent;

typedef enum {
  DRIVE_FSM_INPUT_EVENT_NEUTRAL = NUM_POWER_MAIN_SEQUENCE_EVENTS,  // 31
  DRIVE_FSM_INPUT_EVENT_PARKING,
  DRIVE_FSM_INPUT_EVENT_REVERSE,
  DRIVE_FSM_INPUT_EVENT_CHARGING,
  DRIVE_FSM_INPUT_EVENT_DRIVE,
  DRIVE_FSM_INPUT_EVENT_FAULT,
  DRIVE_FSM_INPUT_EVENT_FAULT_RECOVER_EBRAKE_PRESSED,
  DRIVE_FSM_INPUT_EVENT_FAULT_RECOVER_RELEASED,
  DRIVE_FSM_INPUT_EVENT_BEGIN_PRECHARGE,
  DRIVE_FSM_INPUT_EVENT_PRECHARGE_COMPLETED,
  DRIVE_FSM_INPUT_EVENT_DISCHARGE_COMPLETED,
  DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_CLOSED_DESTINATION_DRIVE,
  DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_CLOSED_DESTINATION_REVERSE,  // 35
  DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_OPENED_DESTINATION_NEUTRAL_PARKING,
  DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_PRESSED,
  DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_RELEASED,
  DRIVE_FSM_INPUT_EVENT_MCI_SET_OUTPUT_DESTINATION_DRIVE,
  DRIVE_FSM_INPUT_EVENT_MCI_SET_OUTPUT_DESTINATION_REVERSE,
  DRIVE_FSM_INPUT_EVENT_MCI_SET_OUTPUT_DESTINATION_OFF,
  NUM_DRIVE_FSM_INPUT_EVENTS
} DriveFsmInputEvent;

typedef enum {
  DRIVE_FSM_OUTPUT_EVENT_DRIVE = NUM_DRIVE_FSM_INPUT_EVENTS,
  DRIVE_FSM_OUTPUT_EVENT_REVERSE,
  DRIVE_FSM_OUTPUT_EVENT_FAULT,
  DRIVE_FSM_OUTPUT_EVENT_PARKING,
  DRIVE_FSM_OUTPUT_EVENT_NEUTRAL,
  NUM_DRIVE_FSM_OUTPUT_EVENTS
} DriveFsmOutputEvent;

typedef enum {
  PEDAL_MONITOR_RX_TIMED_OUT = NUM_DRIVE_FSM_OUTPUT_EVENTS,
  PEDAL_MONITOR_STATE_CHANGE,
  NUM_PEDAL_MONITOR_EVENTS
} PedalMonitorEvent;

typedef enum {
  BATTERY_HEARTBEAT_EVENT_HEALTH_CHECK_REQUEST = NUM_DRIVE_FSM_OUTPUT_EVENTS,
  NUM_BATTERY_HEARTBEAT_EVENTS
} BatteryHeartbeatEvent;

typedef enum {
  CENTRE_CONSOLE_BUTTON_PRESS_EVENT_DRIVE = NUM_BATTERY_HEARTBEAT_EVENTS,
  CENTRE_CONSOLE_BUTTON_PRESS_EVENT_REVERSE,
  CENTRE_CONSOLE_BUTTON_PRESS_EVENT_POWER,
  CENTRE_CONSOLE_BUTTON_PRESS_EVENT_PARKING,
  CENTRE_CONSOLE_BUTTON_PRESS_EVENT_NEUTRAL,
  CENTRE_CONSOLE_BUTTON_PRESS_EVENT_HAZARD,
  NUM_CENTRE_CONSOLE_BUTTON_PRESS_EVENTS
} CentreConsoleButtonPressEvent;

typedef enum {
  HAZARD_EVENT_ON = NUM_CENTRE_CONSOLE_BUTTON_PRESS_EVENTS,
  HAZARD_EVENT_OFF,
  NUM_HAZARD_EVENTS
} HazardEvent;

typedef enum {
  RACE_SWITCH_EVENT_OFF = NUM_HAZARD_EVENTS,
  RACE_SWITCH_EVENT_ON,
  NUM_RACE_SWITCH_EVENTS
} RaceSwitchEvent;

#define NUM_CENTRE_CONSOLE_EVENTS NUM_RACE_SWITCH_EVENTS
