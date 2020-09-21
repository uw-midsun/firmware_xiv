#pragma once

#include "can_msg_defs.h"
#include "can_unpack_impl.h"

#define CAN_UNPACK_BPS_HEARTBEAT(msg_ptr, status_u8_ptr)                                          \
  can_unpack_impl_u8((msg_ptr), 1, (status_u8_ptr), CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY,         \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_SET_RELAY_STATES(msg_ptr, relay_mask_u16_ptr, relay_state_u16_ptr) \
  can_unpack_impl_u16((msg_ptr), 4, (relay_mask_u16_ptr), (relay_state_u16_ptr),      \
                      CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_POWERTRAIN_HEARTBEAT(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_GET_AUX_STATUS(msg_ptr, aux_bat_ov_flag_u8_ptr, aux_bat_ut_flag_u8_ptr) \
  can_unpack_impl_u8((msg_ptr), 2, (aux_bat_ov_flag_u8_ptr), (aux_bat_ut_flag_u8_ptr),     \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY,  \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_FAULT_SEQUENCE(msg_ptr, sequence_u16_ptr)                   \
  can_unpack_impl_u16((msg_ptr), 2, (sequence_u16_ptr), CAN_UNPACK_IMPL_EMPTY, \
                      CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_POWER_ON_MAIN_SEQUENCE(msg_ptr, sequence_u16_ptr)           \
  can_unpack_impl_u16((msg_ptr), 2, (sequence_u16_ptr), CAN_UNPACK_IMPL_EMPTY, \
                      CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_POWER_OFF_SEQUENCE(msg_ptr, sequence_u16_ptr)               \
  can_unpack_impl_u16((msg_ptr), 2, (sequence_u16_ptr), CAN_UNPACK_IMPL_EMPTY, \
                      CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_POWER_ON_AUX_SEQUENCE(msg_ptr, sequence_u16_ptr)            \
  can_unpack_impl_u16((msg_ptr), 2, (sequence_u16_ptr), CAN_UNPACK_IMPL_EMPTY, \
                      CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_DRIVE_OUTPUT(msg_ptr, drive_output_u16_ptr)                     \
  can_unpack_impl_u16((msg_ptr), 2, (drive_output_u16_ptr), CAN_UNPACK_IMPL_EMPTY, \
                      CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_SET_EBRAKE_STATE(msg_ptr, ebrake_state_u8_ptr)                         \
  can_unpack_impl_u8((msg_ptr), 1, (ebrake_state_u8_ptr), CAN_UNPACK_IMPL_EMPTY,          \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_OVUV_DCDC_AUX(msg_ptr, dcdc_ov_flag_u8_ptr, dcdc_uv_flag_u8_ptr,             \
                                 aux_bat_ov_flag_u8_ptr, aux_bat_uv_flag_u8_ptr)                \
  can_unpack_impl_u8((msg_ptr), 4, (dcdc_ov_flag_u8_ptr), (dcdc_uv_flag_u8_ptr),                \
                     (aux_bat_ov_flag_u8_ptr), (aux_bat_uv_flag_u8_ptr), CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_MC_ERROR_LIMITS(msg_ptr, error_id_u16_ptr, limits_u16_ptr)                    \
  can_unpack_impl_u16((msg_ptr), 4, (error_id_u16_ptr), (limits_u16_ptr), CAN_UNPACK_IMPL_EMPTY, \
                      CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_PEDAL_OUTPUT(msg_ptr, throttle_output_u32_ptr, brake_output_u32_ptr) \
  can_unpack_impl_u32((msg_ptr), 8, (throttle_output_u32_ptr), (brake_output_u32_ptr))

#define CAN_UNPACK_CRUISE_TARGET(msg_ptr, target_speed_u8_ptr)                            \
  can_unpack_impl_u8((msg_ptr), 1, (target_speed_u8_ptr), CAN_UNPACK_IMPL_EMPTY,          \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_BRAKE(msg_ptr, brake_state_u16_ptr)                            \
  can_unpack_impl_u16((msg_ptr), 2, (brake_state_u16_ptr), CAN_UNPACK_IMPL_EMPTY, \
                      CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_FRONT_POWER(msg_ptr, power_bitset_u16_ptr)                      \
  can_unpack_impl_u16((msg_ptr), 2, (power_bitset_u16_ptr), CAN_UNPACK_IMPL_EMPTY, \
                      CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_DRIVE_STATE(msg_ptr, drive_state_u16_ptr)                      \
  can_unpack_impl_u16((msg_ptr), 2, (drive_state_u16_ptr), CAN_UNPACK_IMPL_EMPTY, \
                      CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_LIGHTS_SYNC(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_LIGHTS(msg_ptr, lights_id_u8_ptr, state_u8_ptr)                            \
  can_unpack_impl_u8((msg_ptr), 2, (lights_id_u8_ptr), (state_u8_ptr), CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY,     \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_HORN(msg_ptr, state_u8_ptr)                                                   \
  can_unpack_impl_u8((msg_ptr), 1, (state_u8_ptr), CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY,        \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_GET_CHARGER_CONNECTION_STATE(msg_ptr, is_connected_u8_ptr)             \
  can_unpack_impl_u8((msg_ptr), 1, (is_connected_u8_ptr), CAN_UNPACK_IMPL_EMPTY,          \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_SET_CHARGER_RELAY(msg_ptr, state_u8_ptr)                                      \
  can_unpack_impl_u8((msg_ptr), 1, (state_u8_ptr), CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY,        \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_BEGIN_PRECHARGE(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_PRECHARGE_COMPLETED(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_HAZARD(msg_ptr, state_u8_ptr)                                                 \
  can_unpack_impl_u8((msg_ptr), 1, (state_u8_ptr), CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY,        \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_DISCHARGE_PRECHARGE(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_BATTERY_VT(msg_ptr, module_id_u16_ptr, voltage_u16_ptr, temperature_u16_ptr)    \
  can_unpack_impl_u16((msg_ptr), 6, (module_id_u16_ptr), (voltage_u16_ptr), (temperature_u16_ptr), \
                      CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_BATTERY_AGGREGATE_VC(msg_ptr, voltage_u32_ptr, current_u32_ptr) \
  can_unpack_impl_u32((msg_ptr), 8, (voltage_u32_ptr), (current_u32_ptr))

#define CAN_UNPACK_STATE_TRANSITION_FAULT(msg_ptr, state_machine_u16_ptr, fault_reason_u16_ptr) \
  can_unpack_impl_u16((msg_ptr), 4, (state_machine_u16_ptr), (fault_reason_u16_ptr),            \
                      CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_MOTOR_CONTROLLER_VC(msg_ptr, mc_voltage_1_u16_ptr, mc_current_1_u16_ptr, \
                                       mc_voltage_2_u16_ptr, mc_current_2_u16_ptr)          \
  can_unpack_impl_u16((msg_ptr), 8, (mc_voltage_1_u16_ptr), (mc_current_1_u16_ptr),         \
                      (mc_voltage_2_u16_ptr), (mc_current_2_u16_ptr))

#define CAN_UNPACK_MOTOR_VELOCITY(msg_ptr, vehicle_velocity_left_u16_ptr,      \
                                  vehicle_velocity_right_u16_ptr)              \
  can_unpack_impl_u16((msg_ptr), 4, (vehicle_velocity_left_u16_ptr),           \
                      (vehicle_velocity_right_u16_ptr), CAN_UNPACK_IMPL_EMPTY, \
                      CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_MOTOR_DEBUG(msg_ptr, data_u64_ptr) \
  can_unpack_impl_u64((msg_ptr), 8, (data_u64_ptr))

#define CAN_UNPACK_MOTOR_TEMPS(msg_ptr, motor_temp_l_u32_ptr, motor_temp_r_u32_ptr) \
  can_unpack_impl_u32((msg_ptr), 8, (motor_temp_l_u32_ptr), (motor_temp_r_u32_ptr))

#define CAN_UNPACK_MOTOR_AMP_HR(msg_ptr, motor_amp_hr_l_u32_ptr, motor_amp_hr_r_u32_ptr) \
  can_unpack_impl_u32((msg_ptr), 8, (motor_amp_hr_l_u32_ptr), (motor_amp_hr_r_u32_ptr))

#define CAN_UNPACK_ODOMETER(msg_ptr, odometer_val_u32_ptr) \
  can_unpack_impl_u32((msg_ptr), 4, (odometer_val_u32_ptr), CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_CRUISE_CONTROL_COMMAND(msg_ptr, command_u8_ptr)                                 \
  can_unpack_impl_u8((msg_ptr), 1, (command_u8_ptr), CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY,          \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_AUX_DCDC_VC(msg_ptr, aux_voltage_u16_ptr, aux_current_u16_ptr, \
                               dcdc_voltage_u16_ptr, dcdc_current_u16_ptr)        \
  can_unpack_impl_u16((msg_ptr), 8, (aux_voltage_u16_ptr), (aux_current_u16_ptr), \
                      (dcdc_voltage_u16_ptr), (dcdc_current_u16_ptr))

#define CAN_UNPACK_DCDC_TEMPS(msg_ptr, temp_1_u16_ptr, temp_2_u16_ptr)                         \
  can_unpack_impl_u16((msg_ptr), 4, (temp_1_u16_ptr), (temp_2_u16_ptr), CAN_UNPACK_IMPL_EMPTY, \
                      CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_CHARGER_INFO(msg_ptr, current_u16_ptr, voltage_u16_ptr, status_bitset_u16_ptr)  \
  can_unpack_impl_u16((msg_ptr), 6, (current_u16_ptr), (voltage_u16_ptr), (status_bitset_u16_ptr), \
                      CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_REQUEST_TO_CHARGE(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_ALLOW_CHARGING(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_CHARGER_CONNECTED_STATE(msg_ptr, is_connected_u8_ptr)                  \
  can_unpack_impl_u8((msg_ptr), 1, (is_connected_u8_ptr), CAN_UNPACK_IMPL_EMPTY,          \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_LINEAR_ACCELERATION(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_ANGULAR_ROTATION(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_CHARGER_FAULT(msg_ptr, fault_u8_ptr)                                          \
  can_unpack_impl_u8((msg_ptr), 1, (fault_u8_ptr), CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY,        \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_FRONT_CURRENT_MEASUREMENT(msg_ptr, current_id_u16_ptr, current_u16_ptr) \
  can_unpack_impl_u16((msg_ptr), 4, (current_id_u16_ptr), (current_u16_ptr),               \
                      CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_REAR_CURRENT_MEASUREMENT(msg_ptr, current_id_u16_ptr, current_u16_ptr) \
  can_unpack_impl_u16((msg_ptr), 4, (current_id_u16_ptr), (current_u16_ptr),              \
                      CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_AUX_BATTERY_STATUS(msg_ptr, aux_battery_volt_u16_ptr, aux_battery_temp_u16_ptr, \
                                      dcdc_status_u16_ptr)                                         \
  can_unpack_impl_u16((msg_ptr), 6, (aux_battery_volt_u16_ptr), (aux_battery_temp_u16_ptr),        \
                      (dcdc_status_u16_ptr), CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_BATTERY_FAN_STATE(msg_ptr, fan_1_u8_ptr, fan_2_u8_ptr, fan_3_u8_ptr,            \
                                     fan_4_u8_ptr, fan_5_u8_ptr, fan_6_u8_ptr, fan_7_u8_ptr,       \
                                     fan_8_u8_ptr)                                                 \
  can_unpack_impl_u8((msg_ptr), 8, (fan_1_u8_ptr), (fan_2_u8_ptr), (fan_3_u8_ptr), (fan_4_u8_ptr), \
                     (fan_5_u8_ptr), (fan_6_u8_ptr), (fan_7_u8_ptr), (fan_8_u8_ptr))

#define CAN_UNPACK_BATTERY_RELAY_STATE(msg_ptr, hv_u8_ptr, gnd_u8_ptr)                    \
  can_unpack_impl_u8((msg_ptr), 2, (hv_u8_ptr), (gnd_u8_ptr), CAN_UNPACK_IMPL_EMPTY,      \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_SOLAR_DATA(msg_ptr, data_point_type_u32_ptr, data_value_u32_ptr) \
  can_unpack_impl_u32((msg_ptr), 8, (data_point_type_u32_ptr), (data_value_u32_ptr))

#define CAN_UNPACK_SOLAR_FAULT(msg_ptr, fault_u8_ptr, fault_data_u8_ptr)                       \
  can_unpack_impl_u8((msg_ptr), 2, (fault_u8_ptr), (fault_data_u8_ptr), CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY,      \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)
