#include "power_distribution_gpio_config.h"
#include "power_distribution_events.h"

#define POWER_DISTRIBUTION_I2C_ADDRESS_0 0x74
#define POWER_DISTRIBUTION_I2C_ADDRESS_1 0x76

// Enable pins on front power distribution without dedicated events (probably fine):
// left and right display, main and rear pi, left and right camera, speaker, rear display, spares

// Aliases of pin addresses

// Front power distribution
#define FRONT_PIN_CTR_CONSL_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_5 }
#define FRONT_PIN_STEERING_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_3 }
#define FRONT_PIN_PEDAL_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_1 }
#define FRONT_PIN_MAIN_DISP_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_4 }
#define FRONT_PIN_LEFT_DISPLAY_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_4 }
#define FRONT_PIN_RIGHT_DISPLAY_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_6 }
#define FRONT_PIN_DVR_DISP_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_7 }
#define FRONT_PIN_REAR_DISP_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_5 }
#define FRONT_PIN_LEFT_CAMERA_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_1 }
#define FRONT_PIN_RIGHT_CAMERA_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_3 }
#define FRONT_PIN_MAIN_PI_B_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_0 }
#define FRONT_PIN_REAR_PI_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_2 }
#define FRONT_PIN_HORN_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_7 }
#define FRONT_PIN_SPEAKER_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_0 }
#define FRONT_PIN_FRONT_LEFT_TURN_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_4 }
#define FRONT_PIN_FRONT_RIGHT_TURN_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_2 }
#define FRONT_PIN_DAYTIME_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_5 }
#define FRONT_PIN_PARKING_BRAKE_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_7 }
#define FRONT_PIN_SPARE_1_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_6 }
#define FRONT_PIN_SPARE_2_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_1 }
#define FRONT_PIN_5V_SPARE_1_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_4 }
#define FRONT_PIN_5V_SPARE_2_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_2 }

#define FRONT_PIN_SPARE_2_CTR_CONSL_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_0 }
#define FRONT_PIN_STR_PDL_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_2 }
#define FRONT_PIN_L_R_DVR_DISP_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_5 }
#define FRONT_PIN_DVR_REAR_DISP_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_6 }
#define FRONT_PIN_LEFT_RIGHT_CAM_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_0 }
#define FRONT_PIN_MAIN_REAR_PI_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_1 }
#define FRONT_PIN_FRONT_TURN_LIGHT_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_3 }
#define FRONT_PIN_5V_SPARE_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_3 }

// Rear power distribution
#define REAR_PIN_MOTOR_INTERFACE_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_4 }
#define REAR_PIN_BMS_CARRIER_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_5 }
#define REAR_PIN_SOLAR_SENSE_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_1 }
#define REAR_PIN_TELEMETRY_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_5 }
#define REAR_PIN_CHARGER_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_6 }
#define REAR_PIN_REAR_CAMERA_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_1 }
#define REAR_PIN_REAR_LEFT_TURN_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_7 }
#define REAR_PIN_REAR_RIGHT_TURN_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_5 }
#define REAR_PIN_REAR_LEFT_BRAKEL_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_4 }
#define REAR_PIN_REAR_RIGHT_BRAKEL_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_6 }
#define REAR_PIN_CENTER_BRAKE_LIGHT_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_4 }
#define REAR_PIN_STROBE_LIGHT_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_2 }
#define REAR_PIN_REAR_TURN_LIGHT_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_6 }
#define REAR_PIN_SPARE_1_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_0 }
#define REAR_PIN_SPARE_2_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_2 }
#define REAR_PIN_SPARE_3_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_4 }
#define REAR_PIN_SPARE_4_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_2 }
#define REAR_PIN_SPARE_5_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_7 }
#define REAR_PIN_SPARE_6_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_7 }
#define REAR_PIN_SPARE_7_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_0 }
#define REAR_PIN_SPARE_8_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_1 }
#define REAR_PIN_SPARE_9_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_3 }
#define REAR_PIN_SPARE_10_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_3 }

#define REAR_PIN_SOLAR_TELEMETRY_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_0 }
#define REAR_PIN_REAR_BRAKE_LIGHT_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_5 }
#define REAR_PIN_STROBE_CTR_BRK_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_3 }
#define REAR_PIN_CAM_SPARE_10_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_0 }
#define REAR_PIN_SPARE_1_2_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_1 }
#define REAR_PIN_SPARE_3_4_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_3 }
#define REAR_PIN_SPARE_8_9_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_2 }

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
                      // left display,
                      // right display, rear display, left camera, right camera, main rPi, rear rPi
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
                      // left display,
                      // right display, rear display, main rPi, rear rPi
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
                      // rear display,
                      // main rPi, rear rPi, left camera, right camera, parking brake, daytime
                      // running lights,
                      // front left turn light, front right turn light
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

// rear power distribution:
// on everything: bms carrier, mci, solar sense, telemetry, charger, rear camera
// aux: bms carrier, solar sense, telemetry, charger
// off: nothing
