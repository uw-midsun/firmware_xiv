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

#define CAN_TRANSMIT_REGEN_BRAKING(ack_ptr, state_u8)  \
  ({                                                   \
    CanMessage msg = { 0 };                            \
    CAN_PACK_REGEN_BRAKING(&msg, (state_u8));          \
    StatusCode status = can_transmit(&msg, (ack_ptr)); \
    status;                                            \
  })

#define CAN_TRANSMIT_PEDAL_OUTPUT(throttle_output_u32, brake_output_u32)    \
  ({                                                                        \
    CanMessage msg = { 0 };                                                 \
    CAN_PACK_PEDAL_OUTPUT(&msg, (throttle_output_u32), (brake_output_u32)); \
    StatusCode status = can_transmit(&msg, NULL);                           \
    status;                                                                 \
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

#define CAN_TRANSMIT_MOTOR_TEMPS(motor_temp_l_u32, motor_temp_r_u32)    \
  ({                                                                    \
    CanMessage msg = { 0 };                                             \
    CAN_PACK_MOTOR_TEMPS(&msg, (motor_temp_l_u32), (motor_temp_r_u32)); \
    StatusCode status = can_transmit(&msg, NULL);                       \
    status;                                                             \
  })

#define CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(command_u8)  \
  ({                                                     \
    CanMessage msg = { 0 };                              \
    CAN_PACK_CRUISE_CONTROL_COMMAND(&msg, (command_u8)); \
    StatusCode status = can_transmit(&msg, NULL);        \
    status;                                              \
  })

#define CAN_TRANSMIT_UV_CUTOFF_NOTIFICATION()     \
  ({                                              \
    CanMessage msg = { 0 };                       \
    CAN_PACK_UV_CUTOFF_NOTIFICATION(&msg);        \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_REGEN_BRAKING_TOGGLE_REQUEST() \
  ({                                                \
    CanMessage msg = { 0 };                         \
    CAN_PACK_REGEN_BRAKING_TOGGLE_REQUEST(&msg);    \
    StatusCode status = can_transmit(&msg, NULL);   \
    status;                                         \
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

#define CAN_TRANSMIT_SOLAR_DATA_6_MPPTS(data_point_type_u32, data_value_u32)    \
  ({                                                                            \
    CanMessage msg = { 0 };                                                     \
    CAN_PACK_SOLAR_DATA_6_MPPTS(&msg, (data_point_type_u32), (data_value_u32)); \
    StatusCode status = can_transmit(&msg, NULL);                               \
    status;                                                                     \
  })

#define CAN_TRANSMIT_SOLAR_FAULT_6_MPPTS(fault_u8, fault_data_u8)    \
  ({                                                                 \
    CanMessage msg = { 0 };                                          \
    CAN_PACK_SOLAR_FAULT_6_MPPTS(&msg, (fault_u8), (fault_data_u8)); \
    StatusCode status = can_transmit(&msg, NULL);                    \
    status;                                                          \
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

#define CAN_TRANSMIT_SOLAR_DATA_5_MPPTS(data_point_type_u32, data_value_u32)    \
  ({                                                                            \
    CanMessage msg = { 0 };                                                     \
    CAN_PACK_SOLAR_DATA_5_MPPTS(&msg, (data_point_type_u32), (data_value_u32)); \
    StatusCode status = can_transmit(&msg, NULL);                               \
    status;                                                                     \
  })

#define CAN_TRANSMIT_SOLAR_FAULT_5_MPPTS(fault_u8, fault_data_u8)    \
  ({                                                                 \
    CanMessage msg = { 0 };                                          \
    CAN_PACK_SOLAR_FAULT_5_MPPTS(&msg, (fault_u8), (fault_data_u8)); \
    StatusCode status = can_transmit(&msg, NULL);                    \
    status;                                                          \
  })

#define CAN_TRANSMIT_REAR_PD_FAULT(fault_data_u16, enclosure_temp_data_u16, dcdc_temp_data_u16, \
                                   faulting_output_u16)                                         \
  ({                                                                                            \
    CanMessage msg = { 0 };                                                                     \
    CAN_PACK_REAR_PD_FAULT(&msg, (fault_data_u16), (enclosure_temp_data_u16),                   \
                           (dcdc_temp_data_u16), (faulting_output_u16));                        \
    StatusCode status = can_transmit(&msg, NULL);                                               \
    status;                                                                                     \
  })

#define CAN_TRANSMIT_FRONT_PD_FAULT(fault_data_u16, faulting_output_u16)    \
  ({                                                                        \
    CanMessage msg = { 0 };                                                 \
    CAN_PACK_FRONT_PD_FAULT(&msg, (fault_data_u16), (faulting_output_u16)); \
    StatusCode status = can_transmit(&msg, NULL);                           \
    status;                                                                 \
  })

#define CAN_TRANSMIT_BABYDRIVER(id_u8, data0_u8, data1_u8, data2_u8, data3_u8, data4_u8, data5_u8, \
                                data6_u8)                                                          \
  ({                                                                                               \
    CanMessage msg = { 0 };                                                                        \
    CAN_PACK_BABYDRIVER(&msg, (id_u8), (data0_u8), (data1_u8), (data2_u8), (data3_u8), (data4_u8), \
                        (data5_u8), (data6_u8));                                                   \
    StatusCode status = can_transmit(&msg, NULL);                                                  \
    status;                                                                                        \
  })
