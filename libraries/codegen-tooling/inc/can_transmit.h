#pragma once

#include <stddef.h>

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_pack.h"

#define CAN_TRANSMIT_BPS_HEARTBEAT(ack_ptr, status_u8) \
  ({                                                   \
    CanMessage msg = { 0 };                            \
    CAN_PACK_BPS_HEARTBEAT(&msg, (status_u8));         \
    StatusCode status = can_transmit(&msg, (ack_ptr)); \
    status;                                            \
  })

#define CAN_TRANSMIT_SET_RELAY_STATES(ack_ptr, relay_mask_u16, relay_state_u16) \
  ({                                                                            \
    CanMessage msg = { 0 };                                                     \
    CAN_PACK_SET_RELAY_STATES(&msg, (relay_mask_u16), (relay_state_u16));       \
    StatusCode status = can_transmit(&msg, (ack_ptr));                          \
    status;                                                                     \
  })

#define CAN_TRANSMIT_POWERTRAIN_HEARTBEAT(ack_ptr)     \
  ({                                                   \
    CanMessage msg = { 0 };                            \
    CAN_PACK_POWERTRAIN_HEARTBEAT(&msg);               \
    StatusCode status = can_transmit(&msg, (ack_ptr)); \
    status;                                            \
  })

#define CAN_TRANSMIT_GET_AUX_STATUS(ack_ptr, aux_bat_ov_flag_u8, aux_bat_ut_flag_u8) \
  ({                                                                                 \
    CanMessage msg = { 0 };                                                          \
    CAN_PACK_GET_AUX_STATUS(&msg, (aux_bat_ov_flag_u8), (aux_bat_ut_flag_u8));       \
    StatusCode status = can_transmit(&msg, (ack_ptr));                               \
    status;                                                                          \
  })

#define CAN_TRANSMIT_FAULT_SEQUENCE(ack_ptr, sequence_u16) \
  ({                                                       \
    CanMessage msg = { 0 };                                \
    CAN_PACK_FAULT_SEQUENCE(&msg, (sequence_u16));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));     \
    status;                                                \
  })

#define CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(ack_ptr, sequence_u16) \
  ({                                                               \
    CanMessage msg = { 0 };                                        \
    CAN_PACK_POWER_ON_MAIN_SEQUENCE(&msg, (sequence_u16));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));             \
    status;                                                        \
  })

#define CAN_TRANSMIT_POWER_OFF_SEQUENCE(ack_ptr, sequence_u16) \
  ({                                                           \
    CanMessage msg = { 0 };                                    \
    CAN_PACK_POWER_OFF_SEQUENCE(&msg, (sequence_u16));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));         \
    status;                                                    \
  })

#define CAN_TRANSMIT_POWER_ON_AUX_SEQUENCE(ack_ptr, sequence_u16) \
  ({                                                              \
    CanMessage msg = { 0 };                                       \
    CAN_PACK_POWER_ON_AUX_SEQUENCE(&msg, (sequence_u16));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));            \
    status;                                                       \
  })

#define CAN_TRANSMIT_DRIVE_OUTPUT(ack_ptr, drive_output_u16) \
  ({                                                         \
    CanMessage msg = { 0 };                                  \
    CAN_PACK_DRIVE_OUTPUT(&msg, (drive_output_u16));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));       \
    status;                                                  \
  })

#define CAN_TRANSMIT_SET_EBRAKE_STATE(ack_ptr, ebrake_state_u8) \
  ({                                                            \
    CanMessage msg = { 0 };                                     \
    CAN_PACK_SET_EBRAKE_STATE(&msg, (ebrake_state_u8));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));          \
    status;                                                     \
  })

#define CAN_TRANSMIT_OVUV_DCDC_AUX(dcdc_ov_flag_u8, dcdc_uv_flag_u8, aux_bat_ov_flag_u8,     \
                                   aux_bat_uv_flag_u8)                                       \
  ({                                                                                         \
    CanMessage msg = { 0 };                                                                  \
    CAN_PACK_OVUV_DCDC_AUX(&msg, (dcdc_ov_flag_u8), (dcdc_uv_flag_u8), (aux_bat_ov_flag_u8), \
                           (aux_bat_uv_flag_u8));                                            \
    StatusCode status = can_transmit(&msg, NULL);                                            \
    status;                                                                                  \
  })

#define CAN_TRANSMIT_MC_ERROR_LIMITS(error_id_u16, limits_u16)    \
  ({                                                              \
    CanMessage msg = { 0 };                                       \
    CAN_PACK_MC_ERROR_LIMITS(&msg, (error_id_u16), (limits_u16)); \
    StatusCode status = can_transmit(&msg, NULL);                 \
    status;                                                       \
  })

#define CAN_TRANSMIT_PEDAL_OUTPUT(throttle_output_u32, brake_output_u32)    \
  ({                                                                        \
    CanMessage msg = { 0 };                                                 \
    CAN_PACK_PEDAL_OUTPUT(&msg, (throttle_output_u32), (brake_output_u32)); \
    StatusCode status = can_transmit(&msg, NULL);                           \
    status;                                                                 \
  })

#define CAN_TRANSMIT_CRUISE_TARGET(target_speed_u8)  \
  ({                                                 \
    CanMessage msg = { 0 };                          \
    CAN_PACK_CRUISE_TARGET(&msg, (target_speed_u8)); \
    StatusCode status = can_transmit(&msg, NULL);    \
    status;                                          \
  })

#define CAN_TRANSMIT_BRAKE(brake_state_u16)       \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_BRAKE(&msg, (brake_state_u16));      \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_FRONT_POWER(power_bitset_u16)  \
  ({                                                \
    CanMessage msg = { 0 };                         \
    CAN_PACK_FRONT_POWER(&msg, (power_bitset_u16)); \
    StatusCode status = can_transmit(&msg, NULL);   \
    status;                                         \
  })

#define CAN_TRANSMIT_DRIVE_STATE(drive_state_u16)  \
  ({                                               \
    CanMessage msg = { 0 };                        \
    CAN_PACK_DRIVE_STATE(&msg, (drive_state_u16)); \
    StatusCode status = can_transmit(&msg, NULL);  \
    status;                                        \
  })

#define CAN_TRANSMIT_LIGHTS_SYNC()                \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_LIGHTS_SYNC(&msg);                   \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_LIGHTS(lights_id_u8, state_u8)    \
  ({                                                   \
    CanMessage msg = { 0 };                            \
    CAN_PACK_LIGHTS(&msg, (lights_id_u8), (state_u8)); \
    StatusCode status = can_transmit(&msg, NULL);      \
    status;                                            \
  })

#define CAN_TRANSMIT_HORN(state_u8)               \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_HORN(&msg, (state_u8));              \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_GET_CHARGER_CONNECTION_STATE(is_connected_u8)  \
  ({                                                                \
    CanMessage msg = { 0 };                                         \
    CAN_PACK_GET_CHARGER_CONNECTION_STATE(&msg, (is_connected_u8)); \
    StatusCode status = can_transmit(&msg, NULL);                   \
    status;                                                         \
  })

#define CAN_TRANSMIT_SET_CHARGER_RELAY(state_u8)  \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_SET_CHARGER_RELAY(&msg, (state_u8)); \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_BEGIN_PRECHARGE()            \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_BEGIN_PRECHARGE(&msg);               \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_PRECHARGE_COMPLETED()        \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_PRECHARGE_COMPLETED(&msg);           \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_HAZARD(state_u8)             \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_HAZARD(&msg, (state_u8));            \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_DISCHARGE_PRECHARGE()        \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_DISCHARGE_PRECHARGE(&msg);           \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_BATTERY_VT(module_id_u16, voltage_u16, temperature_u16)      \
  ({                                                                              \
    CanMessage msg = { 0 };                                                       \
    CAN_PACK_BATTERY_VT(&msg, (module_id_u16), (voltage_u16), (temperature_u16)); \
    StatusCode status = can_transmit(&msg, NULL);                                 \
    status;                                                                       \
  })

#define CAN_TRANSMIT_BATTERY_AGGREGATE_VC(voltage_u32, current_u32)    \
  ({                                                                   \
    CanMessage msg = { 0 };                                            \
    CAN_PACK_BATTERY_AGGREGATE_VC(&msg, (voltage_u32), (current_u32)); \
    StatusCode status = can_transmit(&msg, NULL);                      \
    status;                                                            \
  })

#define CAN_TRANSMIT_STATE_TRANSITION_FAULT(state_machine_u16, fault_reason_u16)    \
  ({                                                                                \
    CanMessage msg = { 0 };                                                         \
    CAN_PACK_STATE_TRANSITION_FAULT(&msg, (state_machine_u16), (fault_reason_u16)); \
    StatusCode status = can_transmit(&msg, NULL);                                   \
    status;                                                                         \
  })

#define CAN_TRANSMIT_MOTOR_CONTROLLER_VC(mc_voltage_1_u16, mc_current_1_u16, mc_voltage_2_u16,     \
                                         mc_current_2_u16)                                         \
  ({                                                                                               \
    CanMessage msg = { 0 };                                                                        \
    CAN_PACK_MOTOR_CONTROLLER_VC(&msg, (mc_voltage_1_u16), (mc_current_1_u16), (mc_voltage_2_u16), \
                                 (mc_current_2_u16));                                              \
    StatusCode status = can_transmit(&msg, NULL);                                                  \
    status;                                                                                        \
  })

#define CAN_TRANSMIT_MOTOR_VELOCITY(vehicle_velocity_left_u16, vehicle_velocity_right_u16)    \
  ({                                                                                          \
    CanMessage msg = { 0 };                                                                   \
    CAN_PACK_MOTOR_VELOCITY(&msg, (vehicle_velocity_left_u16), (vehicle_velocity_right_u16)); \
    StatusCode status = can_transmit(&msg, NULL);                                             \
    status;                                                                                   \
  })

#define CAN_TRANSMIT_MOTOR_DEBUG(data_u64)        \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_MOTOR_DEBUG(&msg, (data_u64));       \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_MOTOR_TEMPS(motor_temp_l_u32, motor_temp_r_u32)    \
  ({                                                                    \
    CanMessage msg = { 0 };                                             \
    CAN_PACK_MOTOR_TEMPS(&msg, (motor_temp_l_u32), (motor_temp_r_u32)); \
    StatusCode status = can_transmit(&msg, NULL);                       \
    status;                                                             \
  })

#define CAN_TRANSMIT_MOTOR_AMP_HR(motor_amp_hr_l_u32, motor_amp_hr_r_u32)    \
  ({                                                                         \
    CanMessage msg = { 0 };                                                  \
    CAN_PACK_MOTOR_AMP_HR(&msg, (motor_amp_hr_l_u32), (motor_amp_hr_r_u32)); \
    StatusCode status = can_transmit(&msg, NULL);                            \
    status;                                                                  \
  })

#define CAN_TRANSMIT_ODOMETER(odometer_val_u32)   \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_ODOMETER(&msg, (odometer_val_u32));  \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(command_u8)  \
  ({                                                     \
    CanMessage msg = { 0 };                              \
    CAN_PACK_CRUISE_CONTROL_COMMAND(&msg, (command_u8)); \
    StatusCode status = can_transmit(&msg, NULL);        \
    status;                                              \
  })

#define CAN_TRANSMIT_AUX_DCDC_VC(aux_voltage_u16, aux_current_u16, dcdc_voltage_u16,     \
                                 dcdc_current_u16)                                       \
  ({                                                                                     \
    CanMessage msg = { 0 };                                                              \
    CAN_PACK_AUX_DCDC_VC(&msg, (aux_voltage_u16), (aux_current_u16), (dcdc_voltage_u16), \
                         (dcdc_current_u16));                                            \
    StatusCode status = can_transmit(&msg, NULL);                                        \
    status;                                                                              \
  })

#define CAN_TRANSMIT_DCDC_TEMPS(temp_1_u16, temp_2_u16)    \
  ({                                                       \
    CanMessage msg = { 0 };                                \
    CAN_PACK_DCDC_TEMPS(&msg, (temp_1_u16), (temp_2_u16)); \
    StatusCode status = can_transmit(&msg, NULL);          \
    status;                                                \
  })

#define CAN_TRANSMIT_CHARGER_INFO(current_u16, voltage_u16, status_bitset_u16)      \
  ({                                                                                \
    CanMessage msg = { 0 };                                                         \
    CAN_PACK_CHARGER_INFO(&msg, (current_u16), (voltage_u16), (status_bitset_u16)); \
    StatusCode status = can_transmit(&msg, NULL);                                   \
    status;                                                                         \
  })

#define CAN_TRANSMIT_REQUEST_TO_CHARGE()          \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_REQUEST_TO_CHARGE(&msg);             \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_ALLOW_CHARGING()             \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_ALLOW_CHARGING(&msg);                \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_CHARGER_CONNECTED_STATE(is_connected_u8)  \
  ({                                                           \
    CanMessage msg = { 0 };                                    \
    CAN_PACK_CHARGER_CONNECTED_STATE(&msg, (is_connected_u8)); \
    StatusCode status = can_transmit(&msg, NULL);              \
    status;                                                    \
  })

#define CAN_TRANSMIT_LINEAR_ACCELERATION()        \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_LINEAR_ACCELERATION(&msg);           \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_ANGULAR_ROTATION()           \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_ANGULAR_ROTATION(&msg);              \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_CHARGER_FAULT(fault_u8)      \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_CHARGER_FAULT(&msg, (fault_u8));     \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_FRONT_CURRENT_MEASUREMENT(current_id_u16, current_u16)    \
  ({                                                                           \
    CanMessage msg = { 0 };                                                    \
    CAN_PACK_FRONT_CURRENT_MEASUREMENT(&msg, (current_id_u16), (current_u16)); \
    StatusCode status = can_transmit(&msg, NULL);                              \
    status;                                                                    \
  })

#define CAN_TRANSMIT_REAR_CURRENT_MEASUREMENT(current_id_u16, current_u16)    \
  ({                                                                          \
    CanMessage msg = { 0 };                                                   \
    CAN_PACK_REAR_CURRENT_MEASUREMENT(&msg, (current_id_u16), (current_u16)); \
    StatusCode status = can_transmit(&msg, NULL);                             \
    status;                                                                   \
  })

#define CAN_TRANSMIT_AUX_BATTERY_STATUS(aux_battery_volt_u16, aux_battery_temp_u16,   \
                                        dcdc_status_u16)                              \
  ({                                                                                  \
    CanMessage msg = { 0 };                                                           \
    CAN_PACK_AUX_BATTERY_STATUS(&msg, (aux_battery_volt_u16), (aux_battery_temp_u16), \
                                (dcdc_status_u16));                                   \
    StatusCode status = can_transmit(&msg, NULL);                                     \
    status;                                                                           \
  })

#define CAN_TRANSMIT_BATTERY_FAN_STATE(fan_1_u8, fan_2_u8, fan_3_u8, fan_4_u8, fan_5_u8, fan_6_u8, \
                                       fan_7_u8, fan_8_u8)                                         \
  ({                                                                                               \
    CanMessage msg = { 0 };                                                                        \
    CAN_PACK_BATTERY_FAN_STATE(&msg, (fan_1_u8), (fan_2_u8), (fan_3_u8), (fan_4_u8), (fan_5_u8),   \
                               (fan_6_u8), (fan_7_u8), (fan_8_u8));                                \
    StatusCode status = can_transmit(&msg, NULL);                                                  \
    status;                                                                                        \
  })

#define CAN_TRANSMIT_BATTERY_RELAY_STATE(hv_u8, gnd_u8)    \
  ({                                                       \
    CanMessage msg = { 0 };                                \
    CAN_PACK_BATTERY_RELAY_STATE(&msg, (hv_u8), (gnd_u8)); \
    StatusCode status = can_transmit(&msg, NULL);          \
    status;                                                \
  })

#define CAN_TRANSMIT_SOLAR_DATA(data_point_type_u32, data_value_u32)    \
  ({                                                                    \
    CanMessage msg = { 0 };                                             \
    CAN_PACK_SOLAR_DATA(&msg, (data_point_type_u32), (data_value_u32)); \
    StatusCode status = can_transmit(&msg, NULL);                       \
    status;                                                             \
  })

#define CAN_TRANSMIT_SOLAR_FAULT(fault_u8, fault_data_u8)    \
  ({                                                         \
    CanMessage msg = { 0 };                                  \
    CAN_PACK_SOLAR_FAULT(&msg, (fault_u8), (fault_data_u8)); \
    StatusCode status = can_transmit(&msg, NULL);            \
    status;                                                  \
  })
