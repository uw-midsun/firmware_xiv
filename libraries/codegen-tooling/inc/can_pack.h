#pragma once

#include "can_msg_defs.h"
#include "can_pack_impl.h"

#define CAN_PACK_BPS_HEARTBEAT(msg_ptr, status_u8)                                                \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_BMS_CARRIER, SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, 1, \
                   (status_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,    \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                 \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SET_RELAY_STATES(msg_ptr, relay_mask_u16, relay_state_u16)                      \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,                                 \
                    SYSTEM_CAN_MESSAGE_SET_RELAY_STATES, 4, (relay_mask_u16), (relay_state_u16), \
                    CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_POWERTRAIN_HEARTBEAT(msg_ptr)                     \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE, \
                      SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT)

#define CAN_PACK_GET_AUX_STATUS(msg_ptr, aux_bat_ov_flag_u8, aux_bat_ut_flag_u8)                   \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE, SYSTEM_CAN_MESSAGE_GET_AUX_STATUS, \
                   2, (aux_bat_ov_flag_u8), (aux_bat_ut_flag_u8), CAN_PACK_IMPL_EMPTY,             \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                  \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_FAULT_SEQUENCE(msg_ptr, sequence_u16)                                         \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,                               \
                    SYSTEM_CAN_MESSAGE_FAULT_SEQUENCE, 2, (sequence_u16), CAN_PACK_IMPL_EMPTY, \
                    CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_POWER_ON_MAIN_SEQUENCE(msg_ptr, sequence_u16)                    \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,                  \
                    SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE, 2, (sequence_u16), \
                    CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_POWER_OFF_SEQUENCE(msg_ptr, sequence_u16)                                         \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,                                   \
                    SYSTEM_CAN_MESSAGE_POWER_OFF_SEQUENCE, 2, (sequence_u16), CAN_PACK_IMPL_EMPTY, \
                    CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_POWER_ON_AUX_SEQUENCE(msg_ptr, sequence_u16)                    \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,                 \
                    SYSTEM_CAN_MESSAGE_POWER_ON_AUX_SEQUENCE, 2, (sequence_u16), \
                    CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_DRIVE_OUTPUT(msg_ptr, drive_output_u16)                                          \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE, SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT, \
                    2, (drive_output_u16), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,              \
                    CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SET_EBRAKE_STATE(msg_ptr, ebrake_state_u8)                                        \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,                                    \
                   SYSTEM_CAN_MESSAGE_SET_EBRAKE_STATE, 1, (ebrake_state_u8), CAN_PACK_IMPL_EMPTY, \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                  \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_OVUV_DCDC_AUX(msg_ptr, dcdc_ov_flag_u8, dcdc_uv_flag_u8, aux_bat_ov_flag_u8, \
                               aux_bat_uv_flag_u8)                                            \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR,                      \
                   SYSTEM_CAN_MESSAGE_OVUV_DCDC_AUX, 4, (dcdc_ov_flag_u8), (dcdc_uv_flag_u8), \
                   (aux_bat_ov_flag_u8), (aux_bat_uv_flag_u8), CAN_PACK_IMPL_EMPTY,           \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_MC_ERROR_LIMITS(msg_ptr, error_id_u16, limits_u16)                      \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,                       \
                    SYSTEM_CAN_MESSAGE_MC_ERROR_LIMITS, 4, (error_id_u16), (limits_u16), \
                    CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_PEDAL_OUTPUT(msg_ptr, throttle_output_u32, brake_output_u32)               \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_PEDAL, SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT, 8, \
                    (throttle_output_u32), (brake_output_u32))

#define CAN_PACK_CRUISE_TARGET(msg_ptr, target_speed_u8)                                       \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_STEERING, SYSTEM_CAN_MESSAGE_CRUISE_TARGET, 1, \
                   (target_speed_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,              \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_BRAKE(msg_ptr, brake_state_u16)                                     \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_PEDAL, SYSTEM_CAN_MESSAGE_BRAKE, 2, \
                    (brake_state_u16), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,     \
                    CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_FRONT_POWER(msg_ptr, power_bitset_u16)                                          \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE, SYSTEM_CAN_MESSAGE_FRONT_POWER, \
                    2, (power_bitset_u16), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,             \
                    CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_DRIVE_STATE(msg_ptr, drive_state_u16)                                             \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER, SYSTEM_CAN_MESSAGE_DRIVE_STATE, \
                    2, (drive_state_u16), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                \
                    CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_LIGHTS_SYNC(msg_ptr)                                       \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR, \
                      SYSTEM_CAN_MESSAGE_LIGHTS_SYNC)

#define CAN_PACK_LIGHTS(msg_ptr, lights_id_u8, state_u8)                                 \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_STEERING, SYSTEM_CAN_MESSAGE_LIGHTS, 2,  \
                   (lights_id_u8), (state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,        \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_HORN(msg_ptr, state_u8)                                                          \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_STEERING, SYSTEM_CAN_MESSAGE_HORN, 1, (state_u8), \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                 \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                 \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_GET_CHARGER_CONNECTION_STATE(msg_ptr, is_connected_u8)                         \
  can_pack_impl_u8(                                                                             \
      (msg_ptr), SYSTEM_CAN_DEVICE_CHARGER, SYSTEM_CAN_MESSAGE_GET_CHARGER_CONNECTION_STATE, 1, \
      (is_connected_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,         \
      CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SET_CHARGER_RELAY(msg_ptr, state_u8)                                        \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,                              \
                   SYSTEM_CAN_MESSAGE_SET_CHARGER_RELAY, 1, (state_u8), CAN_PACK_IMPL_EMPTY, \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,            \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_BEGIN_PRECHARGE(msg_ptr)                          \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE, \
                      SYSTEM_CAN_MESSAGE_BEGIN_PRECHARGE)

#define CAN_PACK_PRECHARGE_COMPLETED(msg_ptr)                        \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER, \
                      SYSTEM_CAN_MESSAGE_PRECHARGE_COMPLETED)

#define CAN_PACK_HAZARD(msg_ptr, state_u8)                                                    \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE, SYSTEM_CAN_MESSAGE_HAZARD, 1, \
                   (state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,             \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_DISCHARGE_PRECHARGE(msg_ptr)                      \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE, \
                      SYSTEM_CAN_MESSAGE_DISCHARGE_PRECHARGE)

#define CAN_PACK_BATTERY_VT(msg_ptr, module_id_u16, voltage_u16, temperature_u16)               \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_BMS_CARRIER, SYSTEM_CAN_MESSAGE_BATTERY_VT, 6, \
                    (module_id_u16), (voltage_u16), (temperature_u16), CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_BATTERY_AGGREGATE_VC(msg_ptr, voltage_u32, current_u32) \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_BMS_CARRIER,            \
                    SYSTEM_CAN_MESSAGE_BATTERY_AGGREGATE_VC, 8, (voltage_u32), (current_u32))

#define CAN_PACK_STATE_TRANSITION_FAULT(msg_ptr, state_machine_u16, fault_reason_u16)  \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,                       \
                    SYSTEM_CAN_MESSAGE_STATE_TRANSITION_FAULT, 4, (state_machine_u16), \
                    (fault_reason_u16), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_MOTOR_CONTROLLER_VC(msg_ptr, mc_voltage_1_u16, mc_current_1_u16,  \
                                     mc_voltage_2_u16, mc_current_2_u16)           \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,                 \
                    SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_VC, 8, (mc_voltage_1_u16), \
                    (mc_current_1_u16), (mc_voltage_2_u16), (mc_current_2_u16))

#define CAN_PACK_MOTOR_VELOCITY(msg_ptr, vehicle_velocity_left_u16, vehicle_velocity_right_u16) \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,                              \
                    SYSTEM_CAN_MESSAGE_MOTOR_VELOCITY, 4, (vehicle_velocity_left_u16),          \
                    (vehicle_velocity_right_u16), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_MOTOR_DEBUG(msg_ptr, data_u64)                                                    \
  can_pack_impl_u64((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER, SYSTEM_CAN_MESSAGE_MOTOR_DEBUG, \
                    8, (data_u64))

#define CAN_PACK_MOTOR_TEMPS(msg_ptr, motor_temp_l_u32, motor_temp_r_u32)                          \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER, SYSTEM_CAN_MESSAGE_MOTOR_TEMPS, \
                    8, (motor_temp_l_u32), (motor_temp_r_u32))

#define CAN_PACK_MOTOR_AMP_HR(msg_ptr, motor_amp_hr_l_u32, motor_amp_hr_r_u32) \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,             \
                    SYSTEM_CAN_MESSAGE_MOTOR_AMP_HR, 8, (motor_amp_hr_l_u32),  \
                    (motor_amp_hr_r_u32))

#define CAN_PACK_ODOMETER(msg_ptr, odometer_val_u32)                                               \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER, SYSTEM_CAN_MESSAGE_ODOMETER, 4, \
                    (odometer_val_u32), CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_CRUISE_CONTROL_COMMAND(msg_ptr, command_u8)                               \
  can_pack_impl_u8(                                                                        \
      (msg_ptr), SYSTEM_CAN_DEVICE_STEERING, SYSTEM_CAN_MESSAGE_CRUISE_CONTROL_COMMAND, 1, \
      (command_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,         \
      CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_AUX_DCDC_VC(msg_ptr, aux_voltage_u16, aux_current_u16, dcdc_voltage_u16,    \
                             dcdc_current_u16)                                               \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR,                    \
                    SYSTEM_CAN_MESSAGE_AUX_DCDC_VC, 8, (aux_voltage_u16), (aux_current_u16), \
                    (dcdc_voltage_u16), (dcdc_current_u16))

#define CAN_PACK_DCDC_TEMPS(msg_ptr, temp_1_u16, temp_2_u16)                      \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR,         \
                    SYSTEM_CAN_MESSAGE_DCDC_TEMPS, 4, (temp_1_u16), (temp_2_u16), \
                    CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SOLAR_DATA_FRONT(msg_ptr, module_id_u16, voltage_u16, current_u16,             \
                                  temperature_u16)                                              \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_SOLAR, SYSTEM_CAN_MESSAGE_SOLAR_DATA_FRONT, 8, \
                    (module_id_u16), (voltage_u16), (current_u16), (temperature_u16))

#define CAN_PACK_CHARGER_INFO(msg_ptr, current_u16, voltage_u16, status_bitset_u16)           \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_CHARGER, SYSTEM_CAN_MESSAGE_CHARGER_INFO, 6, \
                    (current_u16), (voltage_u16), (status_bitset_u16), CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_LINEAR_ACCELERATION(msg_ptr) \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_IMU, SYSTEM_CAN_MESSAGE_LINEAR_ACCELERATION)

#define CAN_PACK_ANGULAR_ROTATION(msg_ptr) \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_IMU, SYSTEM_CAN_MESSAGE_ANGULAR_ROTATION)

#define CAN_PACK_CHARGER_FAULT(msg_ptr, fault_u8)                                             \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHARGER, SYSTEM_CAN_MESSAGE_CHARGER_FAULT, 1, \
                   (fault_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,             \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_FRONT_CURRENT_MEASUREMENT(msg_ptr, current_id_u16, current_u16)       \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT,             \
                    SYSTEM_CAN_MESSAGE_FRONT_CURRENT_MEASUREMENT, 4, (current_id_u16), \
                    (current_u16), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_REAR_CURRENT_MEASUREMENT(msg_ptr, current_id_u16, current_u16)       \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR,             \
                    SYSTEM_CAN_MESSAGE_REAR_CURRENT_MEASUREMENT, 4, (current_id_u16), \
                    (current_u16), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_AUX_BATTERY_STATUS(msg_ptr, aux_battery_volt_u16, aux_battery_temp_u16, \
                                    dcdc_status_u16)                                     \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_POWER_SELECTION,                        \
                    SYSTEM_CAN_MESSAGE_AUX_BATTERY_STATUS, 6, (aux_battery_volt_u16),    \
                    (aux_battery_temp_u16), (dcdc_status_u16), CAN_PACK_IMPL_EMPTY)
