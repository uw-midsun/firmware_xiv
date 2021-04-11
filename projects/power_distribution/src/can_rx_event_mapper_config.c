#include "can_rx_event_mapper_config.h"

#include "can_msg_defs.h"
#include "exported_enums.h"
#include "pd_events.h"

const CanRxEventMapperConfig g_front_can_rx_config = {
  .msg_specs =
      (CanRxEventMapperMsgSpec[]){
          {
              // manual lights control + signals
              .msg_id = SYSTEM_CAN_MESSAGE_LIGHTS,
              .has_type = true,
              .has_state = true,
              .all_types =
                  (uint16_t[]){
                      EE_LIGHT_TYPE_DRL,
                      EE_LIGHT_TYPE_SIGNAL_RIGHT,
                      EE_LIGHT_TYPE_SIGNAL_LEFT,
                      EE_LIGHT_TYPE_SIGNAL_HAZARD,
                  },
              .num_types = 4,
              .type_to_event_id =
                  (EventId[]){
                      [EE_LIGHT_TYPE_DRL] = PD_GPIO_EVENT_DRL,
                      [EE_LIGHT_TYPE_SIGNAL_RIGHT] = PD_SIGNAL_EVENT_RIGHT,
                      [EE_LIGHT_TYPE_SIGNAL_LEFT] = PD_SIGNAL_EVENT_LEFT,
                      [EE_LIGHT_TYPE_SIGNAL_HAZARD] = PD_SIGNAL_EVENT_HAZARD,
                  },
          },
          {
              // horn
              .msg_id = SYSTEM_CAN_MESSAGE_HORN,
              .has_type = false,
              .has_state = true,
              .event_id = PD_GPIO_EVENT_HORN,
          },
          {
              // lights sync
              .msg_id = SYSTEM_CAN_MESSAGE_LIGHTS_SYNC,
              .has_type = false,
              .has_state = false,
              .event_id = PD_SYNC_EVENT_LIGHTS,
          },
          {
              // main power-on sequence
              .msg_id = SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE,
              .has_type = true,
              .has_state = false,
              .all_types =
                  (uint16_t[]){
                      EE_POWER_MAIN_SEQUENCE_TURN_ON_DRIVER_BMS,
                      EE_POWER_MAIN_SEQUENCE_TURN_ON_EVERYTHING,
                  },
              .num_types = 2,
              .type_to_event_id =
                  (EventId[]){
                      [EE_POWER_MAIN_SEQUENCE_TURN_ON_DRIVER_BMS] =
                          PD_POWER_MAIN_SEQUENCE_EVENT_TURN_ON_DRIVER_DISPLAY_BMS,
                      [EE_POWER_MAIN_SEQUENCE_TURN_ON_EVERYTHING] =
                          PD_POWER_MAIN_SEQUENCE_EVENT_TURN_ON_EVERYTHING,
                  },
          },
          {
              // aux power-on sequence
              .msg_id = SYSTEM_CAN_MESSAGE_POWER_ON_AUX_SEQUENCE,
              .has_type = true,
              .has_state = false,
              .all_types =
                  (uint16_t[]){
                      EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING,
                  },
              .num_types = 1,
              .type_to_event_id =
                  (EventId[]){
                      [EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING] =
                          PD_POWER_AUX_SEQUENCE_EVENT_TURN_ON_EVERYTHING,
                  },
          },
          {
              // power-off sequence
              .msg_id = SYSTEM_CAN_MESSAGE_POWER_OFF_SEQUENCE,
              .has_type = true,
              .has_state = false,
              .all_types =
                  (uint16_t[]){
                      EE_POWER_OFF_SEQUENCE_TURN_OFF_EVERYTHING,
                  },
              .num_types = 1,
              .type_to_event_id =
                  (EventId[]){
                      [EE_POWER_OFF_SEQUENCE_TURN_OFF_EVERYTHING] =
                          PD_POWER_OFF_SEQUENCE_EVENT_TURN_OFF_EVERYTHING,
                  },
          },
      },
  .num_msg_specs = 6,
};

const CanRxEventMapperConfig g_rear_can_rx_config = {
  .msg_specs =
      (CanRxEventMapperMsgSpec[]){
          {
              // manual lights control + signals
              .msg_id = SYSTEM_CAN_MESSAGE_LIGHTS,
              .has_type = true,
              .has_state = true,
              .all_types =
                  (uint16_t[]){
                      EE_LIGHT_TYPE_BRAKES,
                      EE_LIGHT_TYPE_STROBE,
                      EE_LIGHT_TYPE_SIGNAL_RIGHT,
                      EE_LIGHT_TYPE_SIGNAL_LEFT,
                      EE_LIGHT_TYPE_SIGNAL_HAZARD,
                  },
              .num_types = 5,
              .type_to_event_id =
                  (EventId[]){
                      [EE_LIGHT_TYPE_BRAKES] = PD_GPIO_EVENT_BRAKE_LIGHT,
                      // note: PD_STROBE_EVENT is normally raised by bps_watcher
                      // this is just for completeness
                      [EE_LIGHT_TYPE_STROBE] = PD_STROBE_EVENT,
                      [EE_LIGHT_TYPE_SIGNAL_RIGHT] = PD_SIGNAL_EVENT_RIGHT,
                      [EE_LIGHT_TYPE_SIGNAL_LEFT] = PD_SIGNAL_EVENT_LEFT,
                      [EE_LIGHT_TYPE_SIGNAL_HAZARD] = PD_SIGNAL_EVENT_HAZARD,
                  },
          },
          {
              // lights sync
              .msg_id = SYSTEM_CAN_MESSAGE_LIGHTS_SYNC,
              .has_type = false,
              .has_state = false,
              .event_id = PD_SYNC_EVENT_LIGHTS,
          },
          {
              // main power-on sequence
              .msg_id = SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE,
              .has_type = true,
              .has_state = false,
              .all_types =
                  (uint16_t[]){
                      EE_POWER_MAIN_SEQUENCE_TURN_ON_DRIVER_BMS,
                      EE_POWER_MAIN_SEQUENCE_TURN_ON_EVERYTHING,
                  },
              .num_types = 2,
              .type_to_event_id =
                  (EventId[]){
                      [EE_POWER_MAIN_SEQUENCE_TURN_ON_DRIVER_BMS] =
                          PD_POWER_MAIN_SEQUENCE_EVENT_TURN_ON_DRIVER_DISPLAY_BMS,
                      [EE_POWER_MAIN_SEQUENCE_TURN_ON_EVERYTHING] =
                          PD_POWER_MAIN_SEQUENCE_EVENT_TURN_ON_EVERYTHING,
                  },
          },
          {
              // aux power-on sequence
              .msg_id = SYSTEM_CAN_MESSAGE_POWER_ON_AUX_SEQUENCE,
              .has_type = true,
              .has_state = false,
              .all_types =
                  (uint16_t[]){
                      EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING,
                  },
              .num_types = 1,
              .type_to_event_id =
                  (EventId[]){
                      [EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING] =
                          PD_POWER_AUX_SEQUENCE_EVENT_TURN_ON_EVERYTHING,
                  },
          },
          {
              // power-off sequence
              .msg_id = SYSTEM_CAN_MESSAGE_POWER_OFF_SEQUENCE,
              .has_type = true,
              .has_state = false,
              .all_types =
                  (uint16_t[]){
                      EE_POWER_OFF_SEQUENCE_TURN_OFF_EVERYTHING,
                  },
              .num_types = 1,
              .type_to_event_id =
                  (EventId[]){
                      [EE_POWER_OFF_SEQUENCE_TURN_OFF_EVERYTHING] =
                          PD_POWER_OFF_SEQUENCE_EVENT_TURN_OFF_EVERYTHING,
                  },
          },
      },
  .num_msg_specs = 5,
};
