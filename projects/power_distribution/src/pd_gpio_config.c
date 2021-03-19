#include "pd_gpio_config.h"
#include "pd_events.h"
#include "pin_defs.h"

// Enable pins on front power distribution without dedicated events (probably fine):
// left and right display, main and rear pi, left and right camera, speaker, rear display, spares

// TODO(SOFT-396): review all of these

const PowerDistributionGpioConfig FRONT_POWER_DISTRIBUTION_GPIO_CONFIG = {
  .events =
      (PowerDistributionGpioEventSpec[]){
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = FRONT_PIN_DRIVER_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_STEERING,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = FRONT_PIN_STEERING_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_CENTRE_CONSOLE,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = FRONT_PIN_CENTRE_CONSOLE_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_DRL,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = FRONT_PIN_DAYTIME_RUNNING_LIGHTS_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_PEDAL,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = FRONT_PIN_PEDAL_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          // TODO(SOFT-396): allow normal gpio for horn
          // {
          //     .event_id = POWER_DISTRIBUTION_GPIO_EVENT_HORN,
          //     .outputs =
          //         (PowerDistributionGpioOutputSpec[]){
          //             {
          //                 .address = FRONT_PIN_HORN_EN,
          //                 .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
          //             },
          //         },
          //     .num_outputs = 1,
          // },
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = FRONT_PIN_FRONT_LEFT_TURN_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_RIGHT,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = FRONT_PIN_FRONT_RIGHT_TURN_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = FRONT_PIN_FRONT_LEFT_TURN_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                      {
                          .address = FRONT_PIN_FRONT_RIGHT_TURN_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 2,
          },
          {
              .event_id = POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_ON_EVERYTHING_MAIN,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      // Turn on: driver display, steering, centre console, pedal, speaker,
                      // left display, right display, infotainment, rear display, left camera, right
                      // camera, main (telemetry/driver display) rPi, fan 1, fan 2
                      {
                          .address = FRONT_PIN_DRIVER_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_STEERING_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_CENTRE_CONSOLE_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_PEDAL_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_SPEAKER_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_LEFT_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_RIGHT_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_INFOTAINMENT_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_REAR_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_MAIN_PI_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_LEFT_CAMERA_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_RIGHT_CAMERA_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                    //   {
                    //       .address = FRONT_PIN_FAN_1_EN,
                    //       .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                    //   },
                    //   {
                    //       .address = FRONT_PIN_FAN_2_EN,
                    //       .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                    //   },
                  },
              .num_outputs = 14,
          },
          {
              .event_id = POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_ON_EVERYTHING_AUX,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      // Turn on: driver display, steering, centre console, pedal, speaker,
                      // rear display, main (telemetry/driver display) rPi, infotainment, fans 1 & 2
                      // Turn off: left camera, right camera, left display, right display
                      // (in case we go from on to aux)
                      {
                          .address = FRONT_PIN_DRIVER_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_MAIN_PI_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_STEERING_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_CENTRE_CONSOLE_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_PEDAL_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_SPEAKER_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_REAR_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_INFOTAINMENT_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                    //   {
                    //       .address = FRONT_PIN_FAN_1_EN,
                    //       .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                    //   },
                    //   {
                    //       .address = FRONT_PIN_FAN_2_EN,
                    //       .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                    //   },
                      {
                          .address = FRONT_PIN_LEFT_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_RIGHT_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_LEFT_CAMERA_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_RIGHT_CAMERA_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                  },
              .num_outputs = 14,
          },
          {
              .event_id = POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_OFF_EVERYTHING,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      // Turn on (or keep on): centre console, pedal
                      // Turn off: driver display, steering, speaker, left display, right display,
                      // infotainment, rear display, main rPi, left camera, right camera, daytime
                      // running lights, front left turn light, front right turn light, fans 1 & 2
                      {
                          .address = FRONT_PIN_CENTRE_CONSOLE_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_PEDAL_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_DRIVER_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_STEERING_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_SPEAKER_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_LEFT_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_RIGHT_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_INFOTAINMENT_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_REAR_DISPLAY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_MAIN_PI_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_LEFT_CAMERA_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_RIGHT_CAMERA_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_DAYTIME_RUNNING_LIGHTS_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_FRONT_LEFT_TURN_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_FRONT_RIGHT_TURN_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                    //   {
                    //       .address = FRONT_PIN_FAN_1_EN,
                    //       .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                    //   },
                    //   {
                    //       .address = FRONT_PIN_FAN_2_EN,
                    //       .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                    //   },
                  },
              .num_outputs = 17,
          },
      },
  .num_events = 11,
};

const PowerDistributionGpioConfig REAR_POWER_DISTRIBUTION_GPIO_CONFIG = {
  .events =
      (PowerDistributionGpioEventSpec[]){
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_BRAKE_LIGHT,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = REAR_PIN_BRAKE_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = REAR_PIN_REAR_LEFT_TURN_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_RIGHT,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = REAR_PIN_REAR_RIGHT_TURN_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = REAR_PIN_REAR_LEFT_TURN_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                      {
                          .address = REAR_PIN_REAR_RIGHT_TURN_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 2,
          },
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_STROBE,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = REAR_PIN_STROBE_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_ON_EVERYTHING_MAIN,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      // Turn on: BMS, MCI, solar sense, charger, rear camera, fan 1 & 2
                      {
                          .address = REAR_PIN_BMS_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_MCI_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_SOLAR_SENSE_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_CHARGER_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_REAR_CAMERA_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_FAN_1_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_FAN_2_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                  },
              .num_outputs = 7,
          },
          {
              .event_id = POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_ON_EVERYTHING_AUX,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      // Turn on: BMS, solar sense, charger
                      // Turn off: MCI, rear camera, fans 1 & 2
                      {
                          .address = REAR_PIN_BMS_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_SOLAR_SENSE_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_CHARGER_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_MCI_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_REAR_CAMERA_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_FAN_1_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_FAN_2_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                  },
              .num_outputs = 7,
          },
          {
              .event_id = POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_OFF_EVERYTHING,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      // Turn on (or keep on): BMS carrier, solar sense, charger
                      // Turn off: MCI, rear camera, brake light, left/right rear turn lights,
                      // fans 1 & 2
                      {
                          .address = REAR_PIN_BMS_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_SOLAR_SENSE_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_CHARGER_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_MCI_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_BRAKE_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_REAR_LEFT_TURN_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_REAR_RIGHT_TURN_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_FAN_1_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_FAN_2_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                  },
              .num_outputs = 9,
          },
      },
  .num_events = 8,
};
