#include "pd_gpio_config.h"
#include "pd_events.h"
#include "pin_defs.h"

#define POWER_DISTRIBUTION_I2C_ADDRESS_0 0x74
#define POWER_DISTRIBUTION_I2C_ADDRESS_1 0x76

// Enable pins on front power distribution without dedicated events (probably fine):
// left and right display, main and rear pi, left and right camera, speaker, rear display, spares

const PowerDistributionGpioConfig FRONT_POWER_DISTRIBUTION_GPIO_CONFIG = {
  .events =
      (PowerDistributionGpioEventSpec[]){
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = FRONT_PIN_DVR_DISP_EN,
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
                          .address = FRONT_PIN_CTR_CONSL_EN,
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
                          .address = FRONT_PIN_DAYTIME_EN,
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
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_HORN,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = FRONT_PIN_HORN_EN,
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
                          .address = FRONT_PIN_FRONT_LEFT_TURN_EN,
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
                          .address = FRONT_PIN_FRONT_RIGHT_TURN_EN,
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
                          .address = FRONT_PIN_FRONT_LEFT_TURN_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                      {
                          .address = FRONT_PIN_FRONT_RIGHT_TURN_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 2,
          },
          {
              .event_id = POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_ON_EVERYTHING_MAIN,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      // Turn on: driver display, steering, centre console, pedal, horn, speaker,
                      // left display, right display, rear display, left camera, right camera, main
                      // rPi, rear rPi
                      {
                          .address = FRONT_PIN_DVR_DISP_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_STEERING_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_CTR_CONSL_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_PEDAL_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_HORN_EN,
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
                          .address = FRONT_PIN_REAR_DISP_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_MAIN_PI_B_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_REAR_PI_EN,
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
                  },
              .num_outputs = 13,
          },
          {
              .event_id = POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_ON_EVERYTHING_AUX,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      // Turn on: driver display, steering, centre console, pedal, horn, speaker,
                      // left display, right display, rear display, main rPi, rear rPi
                      // Turn off: left camera, right camera (in case we go from on to aux)
                      {
                          .address = FRONT_PIN_DVR_DISP_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_STEERING_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_CTR_CONSL_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_PEDAL_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_HORN_EN,
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
                          .address = FRONT_PIN_REAR_DISP_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_MAIN_PI_B_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_REAR_PI_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
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
              .num_outputs = 13,
          },
          {
              .event_id = POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_OFF_EVERYTHING,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      // Turn on (or keep on): centre console, pedal, horn
                      // Turn off: driver display, steering, speaker, left display, right display,
                      // rear display, main rPi, rear rPi, left camera, right camera, parking brake,
                      // daytime running lights, front left turn light, front right turn light
                      {
                          .address = FRONT_PIN_CTR_CONSL_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_PEDAL_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_HORN_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = FRONT_PIN_DVR_DISP_EN,
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
                          .address = FRONT_PIN_REAR_DISP_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_MAIN_PI_B_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_REAR_PI_EN,
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
                          .address = FRONT_PIN_PARKING_BRAKE_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_DAYTIME_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_FRONT_LEFT_TURN_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = FRONT_PIN_FRONT_RIGHT_TURN_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                  },
              .num_outputs = 17,
          },
      },
  .num_events = 12,
};

const PowerDistributionGpioConfig REAR_POWER_DISTRIBUTION_GPIO_CONFIG = {
  .events =
      (PowerDistributionGpioEventSpec[]){
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_BRAKE_LIGHT,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = REAR_PIN_REAR_LEFT_BRAKEL_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                      {
                          .address = REAR_PIN_CENTER_BRAKE_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                      {
                          .address = REAR_PIN_REAR_RIGHT_BRAKEL_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                  },
              .num_outputs = 3,
          },
          {
              .event_id = POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      {
                          .address = REAR_PIN_REAR_LEFT_TURN_EN,
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
                          .address = REAR_PIN_REAR_RIGHT_TURN_EN,
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
                          .address = REAR_PIN_REAR_LEFT_TURN_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                      },
                      {
                          .address = REAR_PIN_REAR_RIGHT_TURN_EN,
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
                      // Turn on: BMS carrier, MCI, solar sense, telemetry, charger, rear camera
                      {
                          .address = REAR_PIN_BMS_CARRIER_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_MOTOR_INTERFACE_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_SOLAR_SENSE_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_TELEMETRY_EN,
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
                  },
              .num_outputs = 6,
          },
          {
              .event_id = POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_ON_EVERYTHING_AUX,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      // Turn on: BMS carrier, solar sense, telemetry, charger
                      // Turn off: MCI, rear camera
                      {
                          .address = REAR_PIN_BMS_CARRIER_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_SOLAR_SENSE_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_TELEMETRY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_CHARGER_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                      },
                      {
                          .address = REAR_PIN_MOTOR_INTERFACE_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_REAR_CAMERA_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                  },
              .num_outputs = 6,
          },
          {
              .event_id = POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_OFF_EVERYTHING,
              .outputs =
                  (PowerDistributionGpioOutputSpec[]){
                      // Turn on (or keep on): BMS carrier, solar sense, charger
                      // Turn off: MCI, telemetry, rear camera, left/right/centre brake lights,
                      // left/right rear turn lights
                      {
                          .address = REAR_PIN_BMS_CARRIER_EN,
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
                          .address = REAR_PIN_MOTOR_INTERFACE_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_TELEMETRY_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_REAR_LEFT_BRAKEL_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_CENTER_BRAKE_LIGHT_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_REAR_RIGHT_BRAKEL_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_REAR_LEFT_TURN_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                      {
                          .address = REAR_PIN_REAR_RIGHT_TURN_EN,
                          .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                      },
                  },
              .num_outputs = 10,
          },
      },
  .num_events = 8,
};
