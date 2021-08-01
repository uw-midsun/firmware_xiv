#include "motor_temperature.h"
#include "mci_broadcast.h"

#include "can_transmit.h"
#include "can_unpack.h"
#include "generic_can.h"
#include "motor_can.h"
#include "soft_timer.h"
#include "status.h"

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

static void prv_temperature_tx(SoftTimerId timer_id, void *context) {
  // Transmit data through CAN periodically
  MotorTemperatureStorage *storage = context;
  MotorTemperatureMeasurements measurements = storage->measurements;

  // Handle DSP temperature
  uint32_t left_dsp = (uint32_t)measurements.dsp_measurements[LEFT_MOTOR_CONTROLLER].dsp_temp_c;
  uint32_t right_dsp = (uint32_t)measurements.dsp_measurements[RIGHT_MOTOR_CONTROLLER].dsp_temp_c;
  CAN_TRANSMIT_MOTOR_TEMPS(left_dsp, right_dsp);

  // Handle motor temperature
  uint32_t left_motor =
      (uint32_t)measurements.sink_motor_measurements[LEFT_MOTOR_CONTROLLER].motor_temp_c;
  uint32_t right_motor =
      (uint32_t)measurements.sink_motor_measurements[RIGHT_MOTOR_CONTROLLER].motor_temp_c;
  CAN_TRANSMIT_MOTOR_TEMPS(left_motor, right_motor);

  // Handle heat sink temperature
  uint32_t left_sink =
      (uint32_t)measurements.sink_motor_measurements[LEFT_MOTOR_CONTROLLER].heatsink_temp_c;
  uint32_t right_sink =
      (uint32_t)measurements.sink_motor_measurements[RIGHT_MOTOR_CONTROLLER].heatsink_temp_c;
  CAN_TRANSMIT_MOTOR_TEMPS(left_sink, right_sink);

  soft_timer_start_millis(MOTOR_TEMPERATURE_TX_PERIOD_MS, prv_temperature_tx, storage, NULL);
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

  return soft_timer_start_millis(MOTOR_TEMPERATURE_TX_PERIOD_MS, prv_temperature_tx, storage, NULL);
}
