#include "publish_data_config.h"

#include <stdint.h>

#include "can_transmit.h"
#include "output.h"
#include "status.h"

static StatusCode prv_publish_front_current_measurement(Output output, uint16_t current_data) {
  return CAN_TRANSMIT_FRONT_CURRENT_MEASUREMENT((uint16_t)output, current_data);
}

static StatusCode prv_publish_rear_current_measurement(Output output, uint16_t current_data) {
  return CAN_TRANSMIT_REAR_CURRENT_MEASUREMENT((uint16_t)output, current_data);
}

const PublishDataConfig g_front_publish_data_config = {
  .transmitter = prv_publish_front_current_measurement,
  .outputs_to_publish =
      (Output[]){
          FRONT_OUTPUT_CENTRE_CONSOLE,
          FRONT_OUTPUT_PEDAL,
          FRONT_OUTPUT_STEERING,
          FRONT_OUTPUT_DRIVER_DISPLAY,
          FRONT_OUTPUT_INFOTAINMENT_DISPLAY,
          FRONT_OUTPUT_REAR_DISPLAY,
          FRONT_OUTPUT_LEFT_DISPLAY,
          FRONT_OUTPUT_RIGHT_DISPLAY,
          FRONT_OUTPUT_LEFT_CAMERA,
          FRONT_OUTPUT_RIGHT_CAMERA,
          FRONT_OUTPUT_MAIN_PI,
          FRONT_OUTPUT_SPEAKER,
          FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT,
          FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT,
          FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS,
          FRONT_OUTPUT_UV_VBAT,
      },
  .num_outputs_to_publish = 16,
};

const PublishDataConfig g_rear_publish_data_config = {
  .transmitter = prv_publish_rear_current_measurement,
  .outputs_to_publish =
      (Output[]){
          REAR_OUTPUT_BMS,
          REAR_OUTPUT_MCI,
          REAR_OUTPUT_CHARGER,
          REAR_OUTPUT_SOLAR_SENSE,
          REAR_OUTPUT_REAR_CAMERA,
          REAR_OUTPUT_FAN_1,
          REAR_OUTPUT_FAN_2,
          REAR_OUTPUT_LEFT_REAR_TURN_LIGHT,
          REAR_OUTPUT_RIGHT_REAR_TURN_LIGHT,
          REAR_OUTPUT_BRAKE_LIGHT,
          REAR_OUTPUT_BPS_STROBE_LIGHT,
      },
  .num_outputs_to_publish = 11,
};
