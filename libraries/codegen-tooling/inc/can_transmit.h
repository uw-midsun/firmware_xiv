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

#define CAN_TRANSMIT_POWER_DISTRIBUTION_FAULT(ack_ptr, reason_u8) \
  ({                                                              \
    CanMessage msg = { 0 };                                       \
    CAN_PACK_POWER_DISTRIBUTION_FAULT(&msg, (reason_u8));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));            \
    status;                                                       \
  })

#define CAN_TRANSMIT_BATTERY_RELAY_MAIN(ack_ptr, relay_state_u8) \
  ({                                                             \
    CanMessage msg = { 0 };                                      \
    CAN_PACK_BATTERY_RELAY_MAIN(&msg, (relay_state_u8));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));           \
    status;                                                      \
  })

#define CAN_TRANSMIT_BATTERY_RELAY_SLAVE(ack_ptr, relay_state_u8) \
  ({                                                              \
    CanMessage msg = { 0 };                                       \
    CAN_PACK_BATTERY_RELAY_SLAVE(&msg, (relay_state_u8));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));            \
    status;                                                       \
  })

#define CAN_TRANSMIT_MOTOR_RELAY(ack_ptr, relay_state_u8) \
  ({                                                      \
    CanMessage msg = { 0 };                               \
    CAN_PACK_MOTOR_RELAY(&msg, (relay_state_u8));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));    \
    status;                                               \
  })

#define CAN_TRANSMIT_SOLAR_RELAY_REAR(ack_ptr, relay_state_u8) \
  ({                                                           \
    CanMessage msg = { 0 };                                    \
    CAN_PACK_SOLAR_RELAY_REAR(&msg, (relay_state_u8));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));         \
    status;                                                    \
  })

#define CAN_TRANSMIT_SOLAR_RELAY_FRONT(ack_ptr, relay_state_u8) \
  ({                                                            \
    CanMessage msg = { 0 };                                     \
    CAN_PACK_SOLAR_RELAY_FRONT(&msg, (relay_state_u8));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));          \
    status;                                                     \
  })

#define CAN_TRANSMIT_POWER_STATE(ack_ptr, power_state_u8) \
  ({                                                      \
    CanMessage msg = { 0 };                               \
    CAN_PACK_POWER_STATE(&msg, (power_state_u8));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));    \
    status;                                               \
  })

#define CAN_TRANSMIT_POWERTRAIN_HEARTBEAT(ack_ptr)     \
  ({                                                   \
    CanMessage msg = { 0 };                            \
    CAN_PACK_POWERTRAIN_HEARTBEAT(&msg);               \
    StatusCode status = can_transmit(&msg, (ack_ptr)); \
    status;                                            \
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

#define CAN_TRANSMIT_DRIVE_OUTPUT(throttle_u16, direction_u16, cruise_control_u16,     \
                                  mechanical_brake_state_u16)                          \
  ({                                                                                   \
    CanMessage msg = { 0 };                                                            \
    CAN_PACK_DRIVE_OUTPUT(&msg, (throttle_u16), (direction_u16), (cruise_control_u16), \
                          (mechanical_brake_state_u16));                               \
    StatusCode status = can_transmit(&msg, NULL);                                      \
    status;                                                                            \
  })

#define CAN_TRANSMIT_CRUISE_TARGET(target_speed_u8)  \
  ({                                                 \
    CanMessage msg = { 0 };                          \
    CAN_PACK_CRUISE_TARGET(&msg, (target_speed_u8)); \
    StatusCode status = can_transmit(&msg, NULL);    \
    status;                                          \
  })

#define CAN_TRANSMIT_FAN_CONTROL(state_u8)        \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_FAN_CONTROL(&msg, (state_u8));       \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_SET_DISCHARGE_BITSET(discharge_bitset_u64)  \
  ({                                                             \
    CanMessage msg = { 0 };                                      \
    CAN_PACK_SET_DISCHARGE_BITSET(&msg, (discharge_bitset_u64)); \
    StatusCode status = can_transmit(&msg, NULL);                \
    status;                                                      \
  })

#define CAN_TRANSMIT_DISCHARGE_STATE(discharge_bitset_u64)  \
  ({                                                        \
    CanMessage msg = { 0 };                                 \
    CAN_PACK_DISCHARGE_STATE(&msg, (discharge_bitset_u64)); \
    StatusCode status = can_transmit(&msg, NULL);           \
    status;                                                 \
  })

#define CAN_TRANSMIT_LIGHTS_SYNC()                \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_LIGHTS_SYNC(&msg);                   \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_LIGHTS_STATE(light_id_u8, light_state_u8)    \
  ({                                                              \
    CanMessage msg = { 0 };                                       \
    CAN_PACK_LIGHTS_STATE(&msg, (light_id_u8), (light_state_u8)); \
    StatusCode status = can_transmit(&msg, NULL);                 \
    status;                                                       \
  })

#define CAN_TRANSMIT_HORN(state_u8)               \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_HORN(&msg, (state_u8));              \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_CHARGER_CONN_STATE(is_connected_u8)  \
  ({                                                      \
    CanMessage msg = { 0 };                               \
    CAN_PACK_CHARGER_CONN_STATE(&msg, (is_connected_u8)); \
    StatusCode status = can_transmit(&msg, NULL);         \
    status;                                               \
  })

#define CAN_TRANSMIT_CHARGER_SET_RELAY_STATE(state_u8)  \
  ({                                                    \
    CanMessage msg = { 0 };                             \
    CAN_PACK_CHARGER_SET_RELAY_STATE(&msg, (state_u8)); \
    StatusCode status = can_transmit(&msg, NULL);       \
    status;                                             \
  })

#define CAN_TRANSMIT_STEERING_EVENT(event_id_u16, data_u16)    \
  ({                                                           \
    CanMessage msg = { 0 };                                    \
    CAN_PACK_STEERING_EVENT(&msg, (event_id_u16), (data_u16)); \
    StatusCode status = can_transmit(&msg, NULL);              \
    status;                                                    \
  })

#define CAN_TRANSMIT_CENTER_CONSOLE_EVENT(event_id_u16, data_u16)    \
  ({                                                                 \
    CanMessage msg = { 0 };                                          \
    CAN_PACK_CENTER_CONSOLE_EVENT(&msg, (event_id_u16), (data_u16)); \
    StatusCode status = can_transmit(&msg, NULL);                    \
    status;                                                          \
  })

#define CAN_TRANSMIT_MOTOR_CONTROLLER_RESET(motor_controller_index_u8)  \
  ({                                                                    \
    CanMessage msg = { 0 };                                             \
    CAN_PACK_MOTOR_CONTROLLER_RESET(&msg, (motor_controller_index_u8)); \
    StatusCode status = can_transmit(&msg, NULL);                       \
    status;                                                             \
  })

#define CAN_TRANSMIT_BATTERY_SOC()                \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_BATTERY_SOC(&msg);                   \
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

#define CAN_TRANSMIT_SOLAR_DATA_FRONT(module_id_u16, voltage_u16, current_u16, temperature_u16) \
  ({                                                                                            \
    CanMessage msg = { 0 };                                                                     \
    CAN_PACK_SOLAR_DATA_FRONT(&msg, (module_id_u16), (voltage_u16), (current_u16),              \
                              (temperature_u16));                                               \
    StatusCode status = can_transmit(&msg, NULL);                                               \
    status;                                                                                     \
  })

#define CAN_TRANSMIT_SOLAR_DATA_REAR(module_id_u16, voltage_u16, current_u16, temperature_u16) \
  ({                                                                                           \
    CanMessage msg = { 0 };                                                                    \
    CAN_PACK_SOLAR_DATA_REAR(&msg, (module_id_u16), (voltage_u16), (current_u16),              \
                             (temperature_u16));                                               \
    StatusCode status = can_transmit(&msg, NULL);                                              \
    status;                                                                                    \
  })

#define CAN_TRANSMIT_CHARGER_INFO(current_u16, voltage_u16, status_bitset_u16)      \
  ({                                                                                \
    CanMessage msg = { 0 };                                                         \
    CAN_PACK_CHARGER_INFO(&msg, (current_u16), (voltage_u16), (status_bitset_u16)); \
    StatusCode status = can_transmit(&msg, NULL);                                   \
    status;                                                                         \
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
