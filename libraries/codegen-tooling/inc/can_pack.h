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

#define CAN_PACK_REGEN_BRAKING(msg_ptr, state_u8)                                                 \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE, SYSTEM_CAN_MESSAGE_REGEN_BRAKING, \
                   1, (state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,  \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                 \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_READY_TO_DRIVE(msg_ptr, ready_state_u8)                                           \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE, SYSTEM_CAN_MESSAGE_READY_TO_DRIVE, \
                   1, (ready_state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                  \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                  \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_PEDAL_OUTPUT(msg_ptr, throttle_output_u32, brake_output_u32)               \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_PEDAL, SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT, 8, \
                    (throttle_output_u32), (brake_output_u32))

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

#define CAN_PACK_MOTOR_STATUS(msg_ptr, motor_status_l_u32, motor_status_r_u32) \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,             \
                    SYSTEM_CAN_MESSAGE_MOTOR_STATUS, 8, (motor_status_l_u32),  \
                    (motor_status_r_u32))

#define CAN_PACK_MOTOR_TEMPS(msg_ptr, motor_temp_l_u32, motor_temp_r_u32)                          \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER, SYSTEM_CAN_MESSAGE_MOTOR_TEMPS, \
                    8, (motor_temp_l_u32), (motor_temp_r_u32))

#define CAN_PACK_CRUISE_CONTROL_COMMAND(msg_ptr, command_u8)                               \
  can_pack_impl_u8(                                                                        \
      (msg_ptr), SYSTEM_CAN_DEVICE_STEERING, SYSTEM_CAN_MESSAGE_CRUISE_CONTROL_COMMAND, 1, \
      (command_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,         \
      CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_AUX_MEAS_MAIN_VOLTAGE(msg_ptr, aux_voltage_u16, aux_current_u16, aux_temp_u16, \
                                       main_voltage_u16)                                        \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_POWER_SELECT,                                  \
                    SYSTEM_CAN_MESSAGE_AUX_MEAS_MAIN_VOLTAGE, 8, (aux_voltage_u16),             \
                    (aux_current_u16), (aux_temp_u16), (main_voltage_u16))

#define CAN_PACK_DCDC_MEAS_MAIN_CURRENT(msg_ptr, dcdc_voltage_u16, dcdc_current_u16,  \
                                        dcdc_temp_u16, main_current_u16)              \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_POWER_SELECT,                        \
                    SYSTEM_CAN_MESSAGE_DCDC_MEAS_MAIN_CURRENT, 8, (dcdc_voltage_u16), \
                    (dcdc_current_u16), (dcdc_temp_u16), (main_current_u16))

#define CAN_PACK_POWER_SELECT_STATUS(msg_ptr, fault_bitset_u16, warning_bitset_u16, \
                                     valid_bitset_u16, cell_voltage_u16)            \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_POWER_SELECT,                      \
                    SYSTEM_CAN_MESSAGE_POWER_SELECT_STATUS, 8, (fault_bitset_u16),  \
                    (warning_bitset_u16), (valid_bitset_u16), (cell_voltage_u16))

#define CAN_PACK_UV_CUTOFF_NOTIFICATION(msg_ptr)                             \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT, \
                      SYSTEM_CAN_MESSAGE_UV_CUTOFF_NOTIFICATION)

#define CAN_PACK_REGEN_BRAKING_TOGGLE_REQUEST(msg_ptr)       \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_STEERING, \
                      SYSTEM_CAN_MESSAGE_REGEN_BRAKING_TOGGLE_REQUEST)

#define CAN_PACK_REQUEST_TO_CHARGE(msg_ptr) \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_CHARGER, SYSTEM_CAN_MESSAGE_REQUEST_TO_CHARGE)

#define CAN_PACK_ALLOW_CHARGING(msg_ptr)                           \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_CENTRE_CONSOLE, \
                      SYSTEM_CAN_MESSAGE_ALLOW_CHARGING)

#define CAN_PACK_CHARGER_CONNECTED_STATE(msg_ptr, is_connected_u8)                         \
  can_pack_impl_u8(                                                                        \
      (msg_ptr), SYSTEM_CAN_DEVICE_CHARGER, SYSTEM_CAN_MESSAGE_CHARGER_CONNECTED_STATE, 1, \
      (is_connected_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,    \
      CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SOLAR_DATA_6_MPPTS(msg_ptr, data_point_type_u32, data_value_u32)    \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_SOLAR_6_MPPTS,                      \
                    SYSTEM_CAN_MESSAGE_SOLAR_DATA_6_MPPTS, 8, (data_point_type_u32), \
                    (data_value_u32))

#define CAN_PACK_SOLAR_FAULT_6_MPPTS(msg_ptr, fault_u8, fault_data_u8)                     \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_SOLAR_6_MPPTS,                             \
                   SYSTEM_CAN_MESSAGE_SOLAR_FAULT_6_MPPTS, 2, (fault_u8), (fault_data_u8), \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,          \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

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

#define CAN_PACK_BATTERY_FAN_STATE(msg_ptr, fan_1_u8, fan_2_u8, fan_3_u8, fan_4_u8, fan_5_u8,      \
                                   fan_6_u8, fan_7_u8, fan_8_u8)                                   \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_BMS_CARRIER, SYSTEM_CAN_MESSAGE_BATTERY_FAN_STATE, \
                   8, (fan_1_u8), (fan_2_u8), (fan_3_u8), (fan_4_u8), (fan_5_u8), (fan_6_u8),      \
                   (fan_7_u8), (fan_8_u8))

#define CAN_PACK_BATTERY_RELAY_STATE(msg_ptr, hv_u8, gnd_u8)                      \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_BMS_CARRIER,                      \
                   SYSTEM_CAN_MESSAGE_BATTERY_RELAY_STATE, 2, (hv_u8), (gnd_u8),  \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SOLAR_DATA_5_MPPTS(msg_ptr, data_point_type_u32, data_value_u32)    \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_SOLAR_5_MPPTS,                      \
                    SYSTEM_CAN_MESSAGE_SOLAR_DATA_5_MPPTS, 8, (data_point_type_u32), \
                    (data_value_u32))

#define CAN_PACK_SOLAR_FAULT_5_MPPTS(msg_ptr, fault_u8, fault_data_u8)                     \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_SOLAR_5_MPPTS,                             \
                   SYSTEM_CAN_MESSAGE_SOLAR_FAULT_5_MPPTS, 2, (fault_u8), (fault_data_u8), \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,          \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_REAR_PD_FAULT(msg_ptr, fault_data_u16, enclosure_temp_data_u16, \
                               dcdc_temp_data_u16, faulting_output_u16)          \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR,        \
                    SYSTEM_CAN_MESSAGE_REAR_PD_FAULT, 8, (fault_data_u16),       \
                    (enclosure_temp_data_u16), (dcdc_temp_data_u16), (faulting_output_u16))

#define CAN_PACK_FRONT_PD_FAULT(msg_ptr, fault_data_u16, faulting_output_u16)                      \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT,                         \
                    SYSTEM_CAN_MESSAGE_FRONT_PD_FAULT, 4, (fault_data_u16), (faulting_output_u16), \
                    CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_BABYDRIVER(msg_ptr, id_u8, data0_u8, data1_u8, data2_u8, data3_u8, data4_u8, \
                            data5_u8, data6_u8)                                               \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_BABYDRIVER, SYSTEM_CAN_MESSAGE_BABYDRIVER, 8, \
                   (id_u8), (data0_u8), (data1_u8), (data2_u8), (data3_u8), (data4_u8),       \
                   (data5_u8), (data6_u8))
