#include "pd_gpio_config.h"

#include "output.h"
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
                          .output = FRONT_OUTPUT_DRIVER_DISPLAY,
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
                          .output = FRONT_OUTPUT_STEERING,
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
                          .output = FRONT_OUTPUT_CENTRE_CONSOLE,
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
                          .output = FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS,
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
                          .output = FRONT_OUTPUT_PEDAL,
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
          //                 .output = FRONT_OUTPUT_HORN,
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
                          .output = FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT,
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
                          .output = FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT,
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
                          .output = FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT,
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
                          .output = FRONT_OUTPUT_DRIVER_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_STEERING,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_CENTRE_CONSOLE,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_PEDAL,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_SPEAKER,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_LEFT_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_INFOTAINMENT_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_REAR_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_MAIN_PI,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_LEFT_CAMERA,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_CAMERA,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      //   {
                      //       .output = FRONT_OUTPUT_FAN_1,
                      //       .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      //   },
                      //   {
                      //       .output = FRONT_OUTPUT_FAN_2,
                      //       .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
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
                          .output = FRONT_OUTPUT_DRIVER_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_MAIN_PI,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_STEERING,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_CENTRE_CONSOLE,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_PEDAL,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_SPEAKER,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_REAR_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_INFOTAINMENT_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      //   {
                      //       .output = FRONT_OUTPUT_FAN_1,
                      //       .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      //   },
                      //   {
                      //       .output = FRONT_OUTPUT_FAN_2,
                      //       .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      //   },
                      {
                          .output = FRONT_OUTPUT_LEFT_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_LEFT_CAMERA,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_CAMERA,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
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
                          .output = FRONT_OUTPUT_CENTRE_CONSOLE,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_PEDAL,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_DRIVER_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_STEERING,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_SPEAKER,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_LEFT_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_INFOTAINMENT_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_REAR_DISPLAY,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_MAIN_PI,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_LEFT_CAMERA,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_CAMERA,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      //   {
                      //       .output = FRONT_OUTPUT_FAN_1,
                      //       .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      //   },
                      //   {
                      //       .output = FRONT_OUTPUT_FAN_2,
                      //       .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
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
                          .output = REAR_OUTPUT_BRAKE_LIGHT,
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
                          .output = REAR_OUTPUT_LEFT_REAR_TURN_LIGHT,
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
                          .output = REAR_OUTPUT_RIGHT_REAR_TURN_LIGHT,
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
                          .output = REAR_OUTPUT_LEFT_REAR_TURN_LIGHT,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                      {
                          .output = REAR_OUTPUT_RIGHT_REAR_TURN_LIGHT,
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
                          .output = REAR_OUTPUT_STROBE_LIGHT,
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
                          .output = REAR_OUTPUT_BMS,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_MCI,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_SOLAR_SENSE,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_CHARGER,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_REAR_CAMERA,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_FAN_1,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_FAN_2,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
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
                          .output = REAR_OUTPUT_BMS,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_SOLAR_SENSE,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_CHARGER,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_MCI,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_REAR_CAMERA,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_FAN_1,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_FAN_2,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
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
                          .output = REAR_OUTPUT_BMS,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_SOLAR_SENSE,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_CHARGER,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_MCI,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_BRAKE_LIGHT,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_LEFT_REAR_TURN_LIGHT,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_RIGHT_REAR_TURN_LIGHT,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_FAN_1,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_FAN_2,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_OFF,
                      },
                  },
              .num_outputs = 9,
          },
      },
  .num_events = 8,
};
