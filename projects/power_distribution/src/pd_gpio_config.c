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
              .event_id = PD_POWER_MAIN_SEQUENCE_EVENT_TURN_ON_DRIVER_DISPLAY_BMS,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          .output = FRONT_OUTPUT_DRIVER_DISPLAY,
                          .state = PD_GPIO_STATE_ON,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = PD_POWER_MAIN_SEQUENCE_EVENT_TURN_ON_EVERYTHING,
              .outputs =
                  (PdGpioOutputSpec[]){
                      // On: centre console, pedal, steering, driver display, infotainment display,
                      // rear/left/right display, left/right camera, main (telemetry/driver display)
                      // pi, speaker, fan
                      {
                          .output = FRONT_OUTPUT_CENTRE_CONSOLE,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_PEDAL,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_STEERING,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          // just to be safe even though it was turned on earlier
                          .output = FRONT_OUTPUT_DRIVER_DISPLAY,
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
                          .output = FRONT_OUTPUT_LEFT_DISPLAY,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_RIGHT_DISPLAY,
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
                          .output = FRONT_OUTPUT_MAIN_PI,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_SPEAKER,
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
                      // On: centre console, pedal, steering, driver display, infotainment display,
                      // main pi, speaker, fan
                      // Off: rear/left/right display, left/right camera
                      {
                          .output = FRONT_OUTPUT_CENTRE_CONSOLE,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_PEDAL,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_STEERING,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_DRIVER_DISPLAY,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_INFOTAINMENT_DISPLAY,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_MAIN_PI,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_SPEAKER,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_FAN,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_REAR_DISPLAY,
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
                      // On: centre console, pedal
                      // Off: steering, driver display, infotainment display, rear/left/right
                      // display, left/right camera, main pi, speaker, front turn lights, daytime
                      // running lights
                      {
                          .output = FRONT_OUTPUT_CENTRE_CONSOLE,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_PEDAL,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = FRONT_OUTPUT_STEERING,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_DRIVER_DISPLAY,
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
                      {
                          .output = FRONT_OUTPUT_MAIN_PI,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_SPEAKER,
                          .state = PD_GPIO_STATE_OFF,
                      },
                      {
                          .output = FRONT_OUTPUT_FAN,
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
                          .output = FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS,
                          .state = PD_GPIO_STATE_OFF,
                      },
                  },
              .num_outputs = 16,
          },
      },
  .num_events = 13,
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
              .event_id = PD_POWER_MAIN_SEQUENCE_EVENT_TURN_ON_DRIVER_DISPLAY_BMS,
              .outputs =
                  (PdGpioOutputSpec[]){
                      {
                          // BMS *should* be on already because we run through the off sequence at
                          // startup, but let's be safe
                          .output = REAR_OUTPUT_BMS,
                          .state = PD_GPIO_STATE_ON,
                      },
                  },
              .num_outputs = 1,
          },
          {
              .event_id = PD_POWER_MAIN_SEQUENCE_EVENT_TURN_ON_EVERYTHING,
              .outputs =
                  (PdGpioOutputSpec[]){
                      // On: BMS, MCI, solar sense, charger, rear camera, fans
                      {
                          .output = REAR_OUTPUT_BMS,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_MCI,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_CHARGER,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_SOLAR_SENSE,
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
                      // On: BMS, solar sense, charger
                      // Off: MCI, rear camera, fans
                      {
                          // just to be safe, even though it should be on from earlier
                          .output = REAR_OUTPUT_BMS,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_CHARGER,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_SOLAR_SENSE,
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
                      // On: BMS, solar sense, charger
                      // Turn off: MCI, rear camera, fans, brake light, left/right rear turn lights
                      {
                          .output = REAR_OUTPUT_BMS,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_CHARGER,
                          .state = PD_GPIO_STATE_ON,
                      },
                      {
                          .output = REAR_OUTPUT_SOLAR_SENSE,
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
                  },
              .num_outputs = 10,
          },
      },
  .num_events = 9,
};
