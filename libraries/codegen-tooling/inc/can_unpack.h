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

#define CAN_UNPACK_REGEN_BRAKING(msg_ptr, state_u8_ptr)                                          \
  can_unpack_impl_u8((msg_ptr), 1, (state_u8_ptr), CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY,        \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_REGEN_BRAKING_TOGGLE_REQUEST(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_READY_TO_DRIVE(msg_ptr, ready_state_u8_ptr)                            \
  can_unpack_impl_u8((msg_ptr), 1, (ready_state_u8_ptr), CAN_UNPACK_IMPL_EMPTY,           \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_PEDAL_OUTPUT(msg_ptr, throttle_output_u32_ptr, brake_output_u32_ptr) \
  can_unpack_impl_u32((msg_ptr), 8, (throttle_output_u32_ptr), (brake_output_u32_ptr))

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

#define CAN_UNPACK_MOTOR_STATUS(msg_ptr, motor_status_l_u32_ptr, motor_status_r_u32_ptr) \
  can_unpack_impl_u32((msg_ptr), 8, (motor_status_l_u32_ptr), (motor_status_r_u32_ptr))

#define CAN_UNPACK_MOTOR_TEMPS(msg_ptr, motor_temp_l_u32_ptr, motor_temp_r_u32_ptr) \
  can_unpack_impl_u32((msg_ptr), 8, (motor_temp_l_u32_ptr), (motor_temp_r_u32_ptr))

#define CAN_UNPACK_CRUISE_CONTROL_COMMAND(msg_ptr, command_u8_ptr)                                 \
  can_unpack_impl_u8((msg_ptr), 1, (command_u8_ptr), CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY,          \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_AUX_MEAS_MAIN_VOLTAGE(msg_ptr, aux_voltage_u16_ptr, aux_current_u16_ptr, \
                                         aux_temp_u16_ptr, main_voltage_u16_ptr)            \
  can_unpack_impl_u16((msg_ptr), 8, (aux_voltage_u16_ptr), (aux_current_u16_ptr),           \
                      (aux_temp_u16_ptr), (main_voltage_u16_ptr))

#define CAN_UNPACK_DCDC_MEAS_MAIN_CURRENT(msg_ptr, dcdc_voltage_u16_ptr, dcdc_current_u16_ptr, \
                                          dcdc_temp_u16_ptr, main_current_u16_ptr)             \
  can_unpack_impl_u16((msg_ptr), 8, (dcdc_voltage_u16_ptr), (dcdc_current_u16_ptr),            \
                      (dcdc_temp_u16_ptr), (main_current_u16_ptr))

#define CAN_UNPACK_POWER_SELECT_STATUS(msg_ptr, fault_bitset_u16_ptr, warning_bitset_u16_ptr, \
                                       valid_bitset_u16_ptr, cell_voltage_u16_ptr)            \
  can_unpack_impl_u16((msg_ptr), 8, (fault_bitset_u16_ptr), (warning_bitset_u16_ptr),         \
                      (valid_bitset_u16_ptr), (cell_voltage_u16_ptr))

#define CAN_UNPACK_UV_CUTOFF_NOTIFICATION(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_REQUEST_TO_CHARGE(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_ALLOW_CHARGING(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_CHARGER_CONNECTED_STATE(msg_ptr, is_connected_u8_ptr)                  \
  can_unpack_impl_u8((msg_ptr), 1, (is_connected_u8_ptr), CAN_UNPACK_IMPL_EMPTY,          \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_SOLAR_DATA_6_MPPTS(msg_ptr, data_point_type_u32_ptr, data_value_u32_ptr) \
  can_unpack_impl_u32((msg_ptr), 8, (data_point_type_u32_ptr), (data_value_u32_ptr))

#define CAN_UNPACK_SOLAR_FAULT_6_MPPTS(msg_ptr, fault_u8_ptr, fault_data_u8_ptr)               \
  can_unpack_impl_u8((msg_ptr), 2, (fault_u8_ptr), (fault_data_u8_ptr), CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY,      \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

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

#define CAN_UNPACK_BATTERY_FAN_STATE(msg_ptr, fan_1_u8_ptr, fan_2_u8_ptr, fan_3_u8_ptr,            \
                                     fan_4_u8_ptr, fan_5_u8_ptr, fan_6_u8_ptr, fan_7_u8_ptr,       \
                                     fan_8_u8_ptr)                                                 \
  can_unpack_impl_u8((msg_ptr), 8, (fan_1_u8_ptr), (fan_2_u8_ptr), (fan_3_u8_ptr), (fan_4_u8_ptr), \
                     (fan_5_u8_ptr), (fan_6_u8_ptr), (fan_7_u8_ptr), (fan_8_u8_ptr))

#define CAN_UNPACK_BATTERY_RELAY_STATE(msg_ptr, hv_u8_ptr, gnd_u8_ptr)                    \
  can_unpack_impl_u8((msg_ptr), 2, (hv_u8_ptr), (gnd_u8_ptr), CAN_UNPACK_IMPL_EMPTY,      \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_SOLAR_DATA_5_MPPTS(msg_ptr, data_point_type_u32_ptr, data_value_u32_ptr) \
  can_unpack_impl_u32((msg_ptr), 8, (data_point_type_u32_ptr), (data_value_u32_ptr))

#define CAN_UNPACK_SOLAR_FAULT_5_MPPTS(msg_ptr, fault_u8_ptr, fault_data_u8_ptr)               \
  can_unpack_impl_u8((msg_ptr), 2, (fault_u8_ptr), (fault_data_u8_ptr), CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY,      \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_REAR_PD_FAULT(msg_ptr, fault_data_u16_ptr, enclosure_temp_data_u16_ptr, \
                                 dcdc_temp_data_u16_ptr, faulting_output_u16_ptr)          \
  can_unpack_impl_u16((msg_ptr), 8, (fault_data_u16_ptr), (enclosure_temp_data_u16_ptr),   \
                      (dcdc_temp_data_u16_ptr), (faulting_output_u16_ptr))

#define CAN_UNPACK_FRONT_PD_FAULT(msg_ptr, fault_data_u16_ptr, faulting_output_u16_ptr) \
  can_unpack_impl_u16((msg_ptr), 4, (fault_data_u16_ptr), (faulting_output_u16_ptr),    \
                      CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_BABYDRIVER(msg_ptr, id_u8_ptr, data0_u8_ptr, data1_u8_ptr, data2_u8_ptr,     \
                              data3_u8_ptr, data4_u8_ptr, data5_u8_ptr, data6_u8_ptr)           \
  can_unpack_impl_u8((msg_ptr), 8, (id_u8_ptr), (data0_u8_ptr), (data1_u8_ptr), (data2_u8_ptr), \
                     (data3_u8_ptr), (data4_u8_ptr), (data5_u8_ptr), (data6_u8_ptr))
