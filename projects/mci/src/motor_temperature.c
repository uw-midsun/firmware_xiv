#include "motor_temperature.h"

#include "can_transmit.h"
#include "can_unpack.h"
#include "generic_can.h"
#include "motor_can.h"
#include "soft_timer.h"
#include "status.h"

// Retreiving temperature from left and right motor should be implemented,
// transmitting the data still needs to be done.

static void prv_handle_sink_temperature_rx(const GenericCanMsg *msg, void *context) {
  // add data to storage
  MotorTemperatureStorage *storage = context;
  WaveSculptorSinkMotorTempMeasurement *measurements =
      storage->measurements.sink_motor_measurements;

  WaveSculptorCanId can_id = { .raw = msg->id };
  WaveSculptorCanData can_data = { .raw = msg->data };

  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    if (can_id.device_id == storage->ids[motor_id]) {
      measurements[motor_id] = can_data.sink_motor_temp_measurement;
    }
  }
}

static void prv_handle_dsp_temperature_rx(const GenericCanMsg *msg, void *context) {
  MotorTemperatureStorage *storage = context;
  WaveSculptorDspTempMeasurement *measurements = storage->measurements.dsp_measurements;

  WaveSculptorCanId can_id = { .raw = msg->id };
  WaveSculptorCanData can_data = { .raw = msg->data };

  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    if (can_id.device_id == storage->ids[motor_id]) {
      measurements[motor_id] = can_data.dsp_temp_measurement;
    }
  }
}

static void prv_temperature_tx(MotorTemperatureStorage *storage) {
  MotorTemperatureMeasurements measurements = storage->measurements;
  // Not finished: transmit CAN message
  return;
}

static void prv_handle_temperature_tx(SoftTimerId timer_id, void *context) {
  // transmit data through CAN periodically
  MotorTemperatureStorage *storage = context;
  prv_temperature_tx(storage);
  soft_timer_start_millis(MOTOR_TEMPERATURE_TX_PERIOD_MS, prv_handle_temperature_tx, storage, NULL);
}

// Not finished: Function needs to be called in main.c
StatusCode motor_temperature_init(MotorTemperatureStorage *storage,
                                  MotorTemperatureSettings *setting) {
  status_ok_or_return(generic_can_register_rx(
      setting->motor_can, prv_handle_sink_temperature_rx, GENERIC_CAN_EMPTY_MASK,
      MOTOR_CAN_RIGHT_SINK_MOTOR_TEMPERATURE_FRAME_ID, false, storage));

  status_ok_or_return(generic_can_register_rx(
      setting->motor_can, prv_handle_sink_temperature_rx, GENERIC_CAN_EMPTY_MASK,
      MOTOR_CAN_LEFT_SINK_MOTOR_TEMPERATURE_FRAME_ID, false, storage));

  status_ok_or_return(generic_can_register_rx(
      setting->motor_can, prv_handle_dsp_temperature_rx, GENERIC_CAN_EMPTY_MASK,
      MOTOR_CAN_RIGHT_DSP_TEMPERATURE_FRAME_ID, false, storage));

  status_ok_or_return(generic_can_register_rx(
      setting->motor_can, prv_handle_dsp_temperature_rx, GENERIC_CAN_EMPTY_MASK,
      MOTOR_CAN_LEFT_DSP_TEMPERATURE_FRAME_ID, false, storage));

  return soft_timer_start_millis(MOTOR_TEMPERATURE_TX_PERIOD_MS, prv_handle_temperature_tx, storage,
                                 NULL);
}
