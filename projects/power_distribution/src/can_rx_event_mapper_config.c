#include "can_rx_event_mapper_config.h"
#include "can_msg_defs.h"
#include "exported_enums.h"
#include "pd_events.h"

const PowerDistributionCanRxEventMapperConfig FRONT_POWER_DISTRIBUTION_CAN_RX_CONFIG = {
  .msg_specs =
      (PowerDistributionCanRxEventMapperMsgSpec[]){
          {
              // manual control of front power outputs
              .msg_id = SYSTEM_CAN_MESSAGE_FRONT_POWER,
              .has_type = true,
              .has_state = true,
              .all_types =
                  (uint16_t[]){
                      EE_FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY,
                      EE_FRONT_POWER_DISTRIBUTION_OUTPUT_STEERING,
                      EE_FRONT_POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE,
                      EE_FRONT_POWER_DISTRIBUTION_OUTPUT_PEDAL,
                      EE_FRONT_POWER_DISTRIBUTION_OUTPUT_DRL,
                  },
              .num_types = 5,
              .type_to_event_id =
                  (EventId[]){
                      [EE_FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY] =
                          POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY,
                      [EE_FRONT_POWER_DISTRIBUTION_OUTPUT_STEERING] =
                          POWER_DISTRIBUTION_GPIO_EVENT_STEERING,
                      [EE_FRONT_POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE] =
                          POWER_DISTRIBUTION_GPIO_EVENT_CENTRE_CONSOLE,
                      [EE_FRONT_POWER_DISTRIBUTION_OUTPUT_PEDAL] =
                          POWER_DISTRIBUTION_GPIO_EVENT_PEDAL,
                      [EE_FRONT_POWER_DISTRIBUTION_OUTPUT_DRL] = POWER_DISTRIBUTION_GPIO_EVENT_DRL,
                  },
              .ack = false,
          },
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
                      [EE_LIGHT_TYPE_DRL] = POWER_DISTRIBUTION_GPIO_EVENT_DRL,
                      [EE_LIGHT_TYPE_SIGNAL_RIGHT] = POWER_DISTRIBUTION_SIGNAL_EVENT_RIGHT,
                      [EE_LIGHT_TYPE_SIGNAL_LEFT] = POWER_DISTRIBUTION_SIGNAL_EVENT_LEFT,
                      [EE_LIGHT_TYPE_SIGNAL_HAZARD] = POWER_DISTRIBUTION_SIGNAL_EVENT_HAZARD,
                  },
              .ack = false,
          },
          {
              // horn
              .msg_id = SYSTEM_CAN_MESSAGE_HORN,
              .has_type = false,
              .has_state = true,
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_HORN,
              .ack = false,
          },
          {
              // lights sync
              .msg_id = SYSTEM_CAN_MESSAGE_LIGHTS_SYNC,
              .has_type = false,
              .has_state = false,
              .event_id = POWER_DISTRIBUTION_SYNC_EVENT_LIGHTS,
              .ack = false,
          },
          {
              // main power-on sequence
              // need to do any others?
              .msg_id = SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE,
              .has_type = true,
              .has_state = false,
              .all_types =
                  (uint16_t[]){
                      EE_POWER_MAIN_SEQUENCE_TURN_ON_EVERYTHING,
                  },
              .num_types = 1,
              .type_to_event_id =
                  (EventId[]){
                      [EE_POWER_MAIN_SEQUENCE_TURN_ON_EVERYTHING] =
                          POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_ON_EVERYTHING_MAIN,
                  },
              .ack = true,
          },
          {
              // aux power-on sequence
              // need to do anything on confirm aux status?
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
                          POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_ON_EVERYTHING_AUX,
                  },
              .ack = true,
          },
          {
              // power-off sequence
              // need to do any others?
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
                          POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_OFF_EVERYTHING,
                  },
              .ack = true,
          },
      },
  .num_msg_specs = 7,
};

const PowerDistributionCanRxEventMapperConfig REAR_POWER_DISTRIBUTION_CAN_RX_CONFIG = {
  .msg_specs =
      (PowerDistributionCanRxEventMapperMsgSpec[]){
          // should there be a SYSTEM_CAN_MESSAGE_REAR_POWER for manual control of rear power
          // outputs
          // a la SYSTEM_CAN_MESSAGE_FRONT_POWER?
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
                      [EE_LIGHT_TYPE_STROBE] = POWER_DISTRIBUTION_STROBE_EVENT,
                      [EE_LIGHT_TYPE_BRAKES] = POWER_DISTRIBUTION_GPIO_EVENT_BRAKE_LIGHT,
                      [EE_LIGHT_TYPE_SIGNAL_RIGHT] = POWER_DISTRIBUTION_SIGNAL_EVENT_RIGHT,
                      [EE_LIGHT_TYPE_SIGNAL_LEFT] = POWER_DISTRIBUTION_SIGNAL_EVENT_LEFT,
                      [EE_LIGHT_TYPE_SIGNAL_HAZARD] = POWER_DISTRIBUTION_SIGNAL_EVENT_HAZARD,
                  },
              .ack = false,
          },
          {
              // lights sync
              .msg_id = SYSTEM_CAN_MESSAGE_LIGHTS_SYNC,
              .has_type = false,
              .has_state = false,
              .event_id = POWER_DISTRIBUTION_SYNC_EVENT_LIGHTS,
              .ack = false,
          },
          {
              // main power-on sequence
              // need to do any others?
              .msg_id = SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE,
              .has_type = true,
              .has_state = false,
              .all_types =
                  (uint16_t[]){
                      EE_POWER_MAIN_SEQUENCE_TURN_ON_EVERYTHING,
                  },
              .num_types = 1,
              .type_to_event_id =
                  (EventId[]){
                      [EE_POWER_MAIN_SEQUENCE_TURN_ON_EVERYTHING] =
                          POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_ON_EVERYTHING_MAIN,
                  },
              .ack = true,
          },
          {
              // aux power-on sequence
              // need to do anything on confirm aux status?
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
                          POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_ON_EVERYTHING_AUX,
                  },
              .ack = true,
          },
          {
              // power-off sequence
              // need to do any others?
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
                          POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_OFF_EVERYTHING,
                  },
              .ack = true,
          },
      },
  .num_msg_specs = 5,
};
