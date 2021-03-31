#include "pd_gpio_config.h"

#include "output.h"
#include "pd_events.h"
#include "pin_defs.h"

// Enable pins on front power distribution without dedicated events (probably fine):
// left and right display, main and rear pi, left and right camera, speaker, rear display, spares

const PdGpioConfig FRONT_PD_GPIO_CONFIG = {
  .events =
      (PdGpioEventSpec[]){
          {
              .event_id = PD_GPIO_EVENT_DRIVER_DISPLAY,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = FRONT_OUTPUT_DRIVER_DISPLAY,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = PD_GPIO_EVENT_STEERING,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = FRONT_OUTPUT_STEERING,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = PD_GPIO_EVENT_CENTRE_CONSOLE,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = FRONT_OUTPUT_CENTRE_CONSOLE,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = PD_GPIO_EVENT_DRL,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = PD_GPIO_EVENT_PEDAL,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = FRONT_OUTPUT_PEDAL,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = PD_GPIO_EVENT_HORN,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = FRONT_OUTPUT_HORN,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = PD_GPIO_EVENT_SIGNAL_LEFT,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = PD_GPIO_EVENT_SIGNAL_RIGHT,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = PD_GPIO_EVENT_SIGNAL_HAZARD,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 2,
          },
          {
              .event_id = PD_POWER_MAIN_SEQUENCE_EVENT_TURN_ON_EVERYTHING,
              .outputs =
                  (PdGpioOutputSpec[]){
                      // Turn on: driver display, steering, centre console, pedal, speaker,
                      // left display, right display, infotainment, rear display, left camera, right
                      // camera, main (telemetry/driver display) rPi, fans
                      {
                          .output = FRONT_OUTPUT_DRIVER_DISPLAY,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_STEERING,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_CENTRE_CONSOLE,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_PEDAL,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_SPEAKER,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_LEFT_DISPLAY,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_DISPLAY,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_INFOTAINMENT_DISPLAY,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_REAR_DISPLAY,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_MAIN_PI,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_LEFT_CAMERA,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_CAMERA,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_FAN,
                          .state = PD_GPIO_STATE_ON,
                      },
                  },
              .num_outputs = 13,
          },
          {
              .event_id = PD_POWER_AUX_SEQUENCE_EVENT_TURN_ON_EVERYTHING,
              .outputs =
                  (PdGpioOutputSpec[]){
                      // Turn on: driver display, steering, centre console, pedal, speaker,
                      // rear display, main (telemetry/driver display) rPi, infotainment, fans
                      // Turn off: left camera, right camera, left display, right display
                      // (in case we go from on to aux)
                      {
                          .output = FRONT_OUTPUT_DRIVER_DISPLAY,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_MAIN_PI,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_STEERING,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_CENTRE_CONSOLE,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_PEDAL,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_SPEAKER,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_REAR_DISPLAY,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_INFOTAINMENT_DISPLAY,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_FAN,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_LEFT_DISPLAY,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_DISPLAY,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_LEFT_CAMERA,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_CAMERA,
                          .state = PD_GPIO_STATE_OFF,
                      },
                  },
              .num_outputs = 13,
          },
          {
              .event_id = PD_POWER_OFF_SEQUENCE_EVENT_TURN_OFF_EVERYTHING,
              .outputs =
                  (PdGpioOutputSpec[]){
                      // Turn on (or keep on): centre console, pedal
                      // Turn off: driver display, steering, speaker, left display, right display,
                      // infotainment, rear display, main rPi, left camera, right camera, daytime
                      // running lights, front left turn light, front right turn light, fan
                      {
                          .output = FRONT_OUTPUT_CENTRE_CONSOLE,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_PEDAL,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_DRIVER_DISPLAY,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_STEERING,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_SPEAKER,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_LEFT_DISPLAY,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_DISPLAY,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_INFOTAINMENT_DISPLAY,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_REAR_DISPLAY,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_MAIN_PI,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_LEFT_CAMERA,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_CAMERA,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_FAN,
                          .state = PD_GPIO_STATE_OFF,
                      },
                  },
              .num_outputs = 16,
          },
      },
  .num_events = 11,
};

const PdGpioConfig REAR_PD_GPIO_CONFIG = {
  .events =
      (PdGpioEventSpec[]){
          {
              .event_id = PD_GPIO_EVENT_BRAKE_LIGHT,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = REAR_OUTPUT_BRAKE_LIGHT,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = PD_GPIO_EVENT_SIGNAL_LEFT,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = REAR_OUTPUT_LEFT_REAR_TURN_LIGHT,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = PD_GPIO_EVENT_SIGNAL_RIGHT,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = REAR_OUTPUT_RIGHT_REAR_TURN_LIGHT,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = PD_GPIO_EVENT_SIGNAL_HAZARD,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = REAR_OUTPUT_LEFT_REAR_TURN_LIGHT,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                      {
                          .output = REAR_OUTPUT_RIGHT_REAR_TURN_LIGHT,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 2,
          },
          {
              .event_id = PD_GPIO_EVENT_STROBE,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = REAR_OUTPUT_STROBE_LIGHT,
                          .state = PD_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = PD_POWER_MAIN_SEQUENCE_EVENT_TURN_ON_EVERYTHING,
              .outputs =
                  (PdGpioOutputSpec[]){
                      // Turn on: BMS, MCI, solar sense, charger, rear camera, fan 1 & 2
                      {
                          .output = REAR_OUTPUT_BMS,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_MCI,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_SOLAR_SENSE,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_CHARGER,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_REAR_CAMERA,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_FAN_1,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_FAN_2,
                          .state = PD_GPIO_STATE_ON,
                      },
                  },
              .num_outputs = 7,
          },
          {
              .event_id = PD_POWER_AUX_SEQUENCE_EVENT_TURN_ON_EVERYTHING,
              .outputs =
                  (PdGpioOutputSpec[]){
                      // Turn on: BMS, solar sense, charger
                      // Turn off: MCI, rear camera, fans 1 & 2
                      {
                          .output = REAR_OUTPUT_BMS,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_SOLAR_SENSE,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_CHARGER,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_MCI,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_REAR_CAMERA,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_FAN_1,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_FAN_2,
                          .state = PD_GPIO_STATE_OFF,
                      },
                  },
              .num_outputs = 7,
          },
          {
              .event_id = PD_POWER_OFF_SEQUENCE_EVENT_TURN_OFF_EVERYTHING,
              .outputs =
                  (PdGpioOutputSpec[]){
                      // Turn on (or keep on): BMS carrier, solar sense, charger
                      // Turn off: MCI, rear camera, brake light, left/right rear turn lights,
                      // fans 1 & 2
                      {
                          .output = REAR_OUTPUT_BMS,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_SOLAR_SENSE,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_CHARGER,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_MCI,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_BRAKE_LIGHT,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_LEFT_REAR_TURN_LIGHT,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_RIGHT_REAR_TURN_LIGHT,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_FAN_1,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = REAR_OUTPUT_FAN_2,
                          .state = PD_GPIO_STATE_OFF,
                      },
                  },
              .num_outputs = 9,
          },
      },
  .num_events = 8,
};
