#pragma once

#include <stdbool.h>

#include "can_msg.h"

// For setting the CAN device
typedef enum {
  SYSTEM_CAN_DEVICE_BOOTLOADER = 0,
  SYSTEM_CAN_DEVICE_BMS_CARRIER = 1,
  SYSTEM_CAN_DEVICE_CENTRE_CONSOLE = 2,
  SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR = 3,
  SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT = 4,
  SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER = 5,
  SYSTEM_CAN_DEVICE_PEDAL = 6,
  SYSTEM_CAN_DEVICE_STEERING = 7,
  SYSTEM_CAN_DEVICE_SOLAR_5_MPPTS = 8,
  SYSTEM_CAN_DEVICE_SOLAR_6_MPPTS = 9,
  SYSTEM_CAN_DEVICE_CHARGER = 10,
  SYSTEM_CAN_DEVICE_IMU = 11,
  SYSTEM_CAN_DEVICE_POWER_SELECT = 12,
  SYSTEM_CAN_DEVICE_BABYDRIVER = 15,
  NUM_SYSTEM_CAN_DEVICES = 14
} SystemCanDevice;

// For setting the CAN message ID
typedef enum {
  SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT = 0,
  SYSTEM_CAN_MESSAGE_SET_RELAY_STATES = 1,
  SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE = 6,
  SYSTEM_CAN_MESSAGE_POWER_OFF_SEQUENCE = 7,
  SYSTEM_CAN_MESSAGE_POWER_ON_AUX_SEQUENCE = 8,
  SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT = 9,
  SYSTEM_CAN_MESSAGE_SET_EBRAKE_STATE = 10,
  SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT = 18,
  SYSTEM_CAN_MESSAGE_DRIVE_STATE = 22,
  SYSTEM_CAN_MESSAGE_LIGHTS_SYNC = 23,
  SYSTEM_CAN_MESSAGE_LIGHTS = 24,
  SYSTEM_CAN_MESSAGE_HORN = 25,
  SYSTEM_CAN_MESSAGE_REGEN_BRAKING = 26,
  SYSTEM_CAN_MESSAGE_BEGIN_PRECHARGE = 28,
  SYSTEM_CAN_MESSAGE_PRECHARGE_COMPLETED = 29,
  SYSTEM_CAN_MESSAGE_HAZARD = 30,
  SYSTEM_CAN_MESSAGE_DISCHARGE_PRECHARGE = 31,
  SYSTEM_CAN_MESSAGE_BATTERY_VT = 32,
  SYSTEM_CAN_MESSAGE_BATTERY_AGGREGATE_VC = 33,
  SYSTEM_CAN_MESSAGE_STATE_TRANSITION_FAULT = 34,
  SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_VC = 35,
  SYSTEM_CAN_MESSAGE_MOTOR_VELOCITY = 36,
  SYSTEM_CAN_MESSAGE_MOTOR_STATUS = 37,
  SYSTEM_CAN_MESSAGE_MOTOR_TEMPS = 38,
  SYSTEM_CAN_MESSAGE_CRUISE_CONTROL_COMMAND = 41,
  SYSTEM_CAN_MESSAGE_AUX_STATUS_MAIN_VOLTAGE = 42,
  SYSTEM_CAN_MESSAGE_DCDC_STATUS_MAIN_CURRENT = 43,
  SYSTEM_CAN_MESSAGE_POWER_SELECT_FAULT = 44,
  SYSTEM_CAN_MESSAGE_UV_CUTOFF_NOTIFICATION = 45,
  SYSTEM_CAN_MESSAGE_REQUEST_TO_CHARGE = 48,
  SYSTEM_CAN_MESSAGE_ALLOW_CHARGING = 49,
  SYSTEM_CAN_MESSAGE_CHARGER_CONNECTED_STATE = 50,
  SYSTEM_CAN_MESSAGE_SOLAR_DATA_6_MPPTS = 51,
  SYSTEM_CAN_MESSAGE_SOLAR_FAULT_6_MPPTS = 52,
  SYSTEM_CAN_MESSAGE_CHARGER_FAULT = 53,
  SYSTEM_CAN_MESSAGE_FRONT_CURRENT_MEASUREMENT = 54,
  SYSTEM_CAN_MESSAGE_REAR_CURRENT_MEASUREMENT = 55,
  SYSTEM_CAN_MESSAGE_BATTERY_FAN_STATE = 57,
  SYSTEM_CAN_MESSAGE_BATTERY_RELAY_STATE = 58,
  SYSTEM_CAN_MESSAGE_SOLAR_DATA_5_MPPTS = 59,
  SYSTEM_CAN_MESSAGE_SOLAR_FAULT_5_MPPTS = 60,
  SYSTEM_CAN_MESSAGE_REAR_PD_FAULT = 61,
  SYSTEM_CAN_MESSAGE_FRONT_PD_FAULT = 62,
  SYSTEM_CAN_MESSAGE_BABYDRIVER = 63,
  NUM_SYSTEM_CAN_MESSAGES = 44
} SystemCanMessage;
