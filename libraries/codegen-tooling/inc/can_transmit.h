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

#define CAN_TRANSMIT_SET_RELAY_STATES(ack_ptr, relay_id_bitset_u8, relay_output_bitset_u8) \
  ({                                                                                       \
    CanMessage msg = { 0 };                                                                \
    CAN_PACK_SET_RELAY_STATES(&msg, (relay_id_bitset_u8), (relay_output_bitset_u8));       \
    StatusCode status = can_transmit(&msg, (ack_ptr));                                     \
    status;                                                                                \
  })

#define CAN_TRANSMIT_POWERTRAIN_HEARTBEAT(ack_ptr)     \
  ({                                                   \
    CanMessage msg = { 0 };                            \
    CAN_PACK_POWERTRAIN_HEARTBEAT(&msg);               \
    StatusCode status = can_transmit(&msg, (ack_ptr)); \
    status;                                            \
  })

#define CAN_TRANSMIT_GET_AUX_STATUS(ack_ptr)           \
  ({                                                   \
    CanMessage msg = { 0 };                            \
    CAN_PACK_GET_AUX_STATUS(&msg);                     \
    StatusCode status = can_transmit(&msg, (ack_ptr)); \
    status;                                            \
  })

#define CAN_TRANSMIT_REAR_POWER(ack_ptr, output_bitset_u16, output_state_u16) \
  ({                                                                          \
    CanMessage msg = { 0 };                                                   \
    CAN_PACK_REAR_POWER(&msg, (output_bitset_u16), (output_state_u16));       \
    StatusCode status = can_transmit(&msg, (ack_ptr));                        \
    status;                                                                   \
  })

#define CAN_TRANSMIT_GET_DC_DC_STATUS(ack_ptr)         \
  ({                                                   \
    CanMessage msg = { 0 };                            \
    CAN_PACK_GET_DC_DC_STATUS(&msg);                   \
    StatusCode status = can_transmit(&msg, (ack_ptr)); \
    status;                                            \
  })

#define CAN_TRANSMIT_START_PRECHARGE(ack_ptr)          \
  ({                                                   \
    CanMessage msg = { 0 };                            \
    CAN_PACK_START_PRECHARGE(&msg);                    \
    StatusCode status = can_transmit(&msg, (ack_ptr)); \
    status;                                            \
  })

#define CAN_TRANSMIT_PRECHARGE_COMPLETE(ack_ptr)       \
  ({                                                   \
    CanMessage msg = { 0 };                            \
    CAN_PACK_PRECHARGE_COMPLETE(&msg);                 \
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

#define CAN_TRANSMIT_THROTTLE_OUTPUT(throttle_u16)  \
  ({                                                \
    CanMessage msg = { 0 };                         \
    CAN_PACK_THROTTLE_OUTPUT(&msg, (throttle_u16)); \
    StatusCode status = can_transmit(&msg, NULL);   \
    status;                                         \
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

#define CAN_TRANSMIT_FRONT_POWER(output_bitset_u16, output_state_u16)    \
  ({                                                                     \
    CanMessage msg = { 0 };                                              \
    CAN_PACK_FRONT_POWER(&msg, (output_bitset_u16), (output_state_u16)); \
    StatusCode status = can_transmit(&msg, NULL);                        \
    status;                                                              \
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

#define CAN_TRANSMIT_DRIVE_STATE(drive_state_u8)  \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_DRIVE_STATE(&msg, (drive_state_u8)); \
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
