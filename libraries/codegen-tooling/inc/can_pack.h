#pragma once

#include "can_msg_defs.h"
#include "can_pack_impl.h"

#define CAN_PACK_BPS_HEARTBEAT(msg_ptr, status_u8)                                             \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_PLUTUS, SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, 1,   \
                   (status_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,              \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_POWER_DISTRIBUTION_FAULT(msg_ptr, reason_u8)                             \
  can_pack_impl_u8(                                                                       \
      (msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_FAULT, 1, \
      (reason_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,         \
      CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_BATTERY_RELAY_MAIN(msg_ptr, relay_state_u8)                                     \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_BATTERY_RELAY_MAIN, 1, \
                   (relay_state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                   \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_BATTERY_RELAY_SLAVE(msg_ptr, relay_state_u8)                                     \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_BATTERY_RELAY_SLAVE, 1, \
                   (relay_state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                    \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                 \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_MOTOR_RELAY(msg_ptr, relay_state_u8)                                     \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_MOTOR_RELAY, 1, \
                   (relay_state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,            \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,         \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SOLAR_RELAY_REAR(msg_ptr, relay_state_u8)                                     \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_SOLAR_RELAY_REAR, 1, \
                   (relay_state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                 \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,              \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SOLAR_RELAY_FRONT(msg_ptr, relay_state_u8)                                     \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_SOLAR_RELAY_FRONT, 1, \
                   (relay_state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                  \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,               \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_POWER_STATE(msg_ptr, power_state_u8)                                        \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_PEDAL,                       \
                   SYSTEM_CAN_MESSAGE_POWER_STATE, 1, (power_state_u8), CAN_PACK_IMPL_EMPTY, \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,            \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_POWERTRAIN_HEARTBEAT(msg_ptr) \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT)

#define CAN_PACK_OVUV_DCDC_AUX(msg_ptr, dcdc_ov_flag_u8, dcdc_uv_flag_u8, aux_bat_ov_flag_u8, \
                               aux_bat_uv_flag_u8)                                            \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_OVUV_DCDC_AUX, 4,   \
                   (dcdc_ov_flag_u8), (dcdc_uv_flag_u8), (aux_bat_ov_flag_u8),                \
                   (aux_bat_uv_flag_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,            \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_MC_ERROR_LIMITS(msg_ptr, error_id_u16, limits_u16)                      \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,                       \
                    SYSTEM_CAN_MESSAGE_MC_ERROR_LIMITS, 4, (error_id_u16), (limits_u16), \
                    CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_DRIVE_OUTPUT(msg_ptr, throttle_u16, direction_u16, cruise_control_u16,  \
                              mechanical_brake_state_u16)                                \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_PEDAL,                  \
                    SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT, 8, (throttle_u16), (direction_u16), \
                    (cruise_control_u16), (mechanical_brake_state_u16))

#define CAN_PACK_CRUISE_TARGET(msg_ptr, target_speed_u8)                                        \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_PEDAL,                          \
                   SYSTEM_CAN_MESSAGE_CRUISE_TARGET, 1, (target_speed_u8), CAN_PACK_IMPL_EMPTY, \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,               \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_FAN_CONTROL(msg_ptr, state_u8)                                               \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_FAN_CONTROL, 1,     \
                   (state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,             \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SET_DISCHARGE_BITSET(msg_ptr, discharge_bitset_u64) \
  can_pack_impl_u64((msg_ptr), SYSTEM_CAN_DEVICE_TELEMETRY,          \
                    SYSTEM_CAN_MESSAGE_SET_DISCHARGE_BITSET, 8, (discharge_bitset_u64))

#define CAN_PACK_DISCHARGE_STATE(msg_ptr, discharge_bitset_u64)                                 \
  can_pack_impl_u64((msg_ptr), SYSTEM_CAN_DEVICE_PLUTUS, SYSTEM_CAN_MESSAGE_DISCHARGE_STATE, 8, \
                    (discharge_bitset_u64))

#define CAN_PACK_LIGHTS_SYNC(msg_ptr) \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_LIGHTS_REAR, SYSTEM_CAN_MESSAGE_LIGHTS_SYNC)

#define CAN_PACK_LIGHTS_STATE(msg_ptr, light_id_u8, light_state_u8)                     \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_PEDAL,                  \
                   SYSTEM_CAN_MESSAGE_LIGHTS_STATE, 2, (light_id_u8), (light_state_u8), \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,       \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_HORN(msg_ptr, state_u8)                                                           \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_PEDAL, SYSTEM_CAN_MESSAGE_HORN, 1, \
                   (state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,      \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                  \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_CHARGER_CONN_STATE(msg_ptr, is_connected_u8)                                      \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHARGER, SYSTEM_CAN_MESSAGE_CHARGER_CONN_STATE, 1, \
                   (is_connected_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                    \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                  \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_CHARGER_SET_RELAY_STATE(msg_ptr, state_u8)                                        \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_CHARGER_SET_RELAY_STATE, \
                   1, (state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,   \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                  \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_STEERING_EVENT(msg_ptr, event_id_u16, data_u16)                      \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_STEERING,            \
                    SYSTEM_CAN_MESSAGE_STEERING_EVENT, 4, (event_id_u16), (data_u16), \
                    CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_CENTER_CONSOLE_EVENT(msg_ptr, event_id_u16, data_u16)                      \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_CENTER_CONSOLE,            \
                    SYSTEM_CAN_MESSAGE_CENTER_CONSOLE_EVENT, 4, (event_id_u16), (data_u16), \
                    CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_MOTOR_CONTROLLER_RESET(msg_ptr, motor_controller_index_u8)                   \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_PEDAL,                        \
                   SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_RESET, 1, (motor_controller_index_u8), \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,             \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,             \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_BATTERY_SOC(msg_ptr) \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_PLUTUS, SYSTEM_CAN_MESSAGE_BATTERY_SOC)

#define CAN_PACK_BATTERY_VT(msg_ptr, module_id_u16, voltage_u16, temperature_u16)          \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_PLUTUS, SYSTEM_CAN_MESSAGE_BATTERY_VT, 6, \
                    (module_id_u16), (voltage_u16), (temperature_u16), CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_BATTERY_AGGREGATE_VC(msg_ptr, voltage_u32, current_u32)                          \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_PLUTUS, SYSTEM_CAN_MESSAGE_BATTERY_AGGREGATE_VC, \
                    8, (voltage_u32), (current_u32))

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

#define CAN_PACK_AUX_DCDC_VC(msg_ptr, aux_voltage_u16, aux_current_u16, dcdc_voltage_u16,  \
                             dcdc_current_u16)                                             \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_AUX_DCDC_VC, 8, \
                    (aux_voltage_u16), (aux_current_u16), (dcdc_voltage_u16), (dcdc_current_u16))

#define CAN_PACK_DCDC_TEMPS(msg_ptr, temp_1_u16, temp_2_u16)                              \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_DCDC_TEMPS, 4, \
                    (temp_1_u16), (temp_2_u16), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SOLAR_DATA_FRONT(msg_ptr, module_id_u16, voltage_u16, current_u16,         \
                                  temperature_u16)                                          \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_SOLAR_MASTER_FRONT,                        \
                    SYSTEM_CAN_MESSAGE_SOLAR_DATA_FRONT, 8, (module_id_u16), (voltage_u16), \
                    (current_u16), (temperature_u16))

#define CAN_PACK_SOLAR_DATA_REAR(msg_ptr, module_id_u16, voltage_u16, current_u16,         \
                                 temperature_u16)                                          \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_SOLAR_MASTER_REAR,                        \
                    SYSTEM_CAN_MESSAGE_SOLAR_DATA_REAR, 8, (module_id_u16), (voltage_u16), \
                    (current_u16), (temperature_u16))

#define CAN_PACK_CHARGER_INFO(msg_ptr, current_u16, voltage_u16, status_bitset_u16)           \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_CHARGER, SYSTEM_CAN_MESSAGE_CHARGER_INFO, 6, \
                    (current_u16), (voltage_u16), (status_bitset_u16), CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_LINEAR_ACCELERATION(msg_ptr)                    \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_SENSOR_BOARD, \
                      SYSTEM_CAN_MESSAGE_LINEAR_ACCELERATION)

#define CAN_PACK_ANGULAR_ROTATION(msg_ptr)                       \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_SENSOR_BOARD, \
                      SYSTEM_CAN_MESSAGE_ANGULAR_ROTATION)
