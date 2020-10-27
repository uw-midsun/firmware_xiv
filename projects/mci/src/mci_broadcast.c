#include "mci_broadcast.h"

#include "motor_can.h"
#include "motor_controller.h"
#include "soft_timer.h"

#include "can_transmit.h"
#include "can_unpack.h"
#include "critical_section.h"

#define M_TO_CM_CONV 100

static void prv_broadcast_speed(MotorControllerBroadcastStorage *storage) {
  float *measurements = storage->measurements.vehicle_velocity;
  CAN_TRANSMIT_MOTOR_VELOCITY((uint16_t)measurements[LEFT_MOTOR_CONTROLLER],
                              (uint16_t)measurements[RIGHT_MOTOR_CONTROLLER]);
}

static void prv_broadcast_bus_measurement(MotorControllerBroadcastStorage *storage) {
  WaveSculptorBusMeasurement *measurements = storage->measurements.bus_measurements;
  CAN_TRANSMIT_MOTOR_CONTROLLER_VC((uint16_t)measurements[LEFT_MOTOR_CONTROLLER].bus_voltage_v,
                                   (uint16_t)measurements[LEFT_MOTOR_CONTROLLER].bus_current_a,
                                   (uint16_t)measurements[RIGHT_MOTOR_CONTROLLER].bus_voltage_v,
                                   (uint16_t)measurements[RIGHT_MOTOR_CONTROLLER].bus_current_a);
}

/*
static void prv_handle_speed_rx(const GenericCanMsg *msg, void *context) {
  MotorControllerBroadcastStorage *storage = context;
  float *measurements = storage->measurements.vehicle_velocity;

  WaveSculptorCanId can_id = { .raw = msg->id };
  WaveSculptorCanData can_data = { .raw = msg->data };

  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    if (can_id.device_id == storage->ids[motor_id]) {
      bool disabled = critical_section_start();
      measurements[motor_id] = can_data.velocity_measurement.vehicle_velocity_ms * M_TO_CM_CONV;
      storage->velocity_rx_bitset |= 1 << motor_id;
      critical_section_end(disabled);
      break;
    }
  }
}
*/
/*
static void prv_handle_bus_measurement_rx(const GenericCanMsg *msg, void *context) {
  MotorControllerBroadcastStorage *storage = context;
  WaveSculptorBusMeasurement *measurements = storage->measurements.bus_measurements;

  WaveSculptorCanId can_id = { .raw = msg->id };
  WaveSculptorCanData can_data = { .raw = msg->data };

  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    if (can_id.device_id == storage->ids[motor_id]) {
      bool disabled = critical_section_start();
      measurements[motor_id] = can_data.bus_measurement;
      storage->bus_rx_bitset |= 1 << motor_id;
      critical_section_end(disabled);
    }
  }
}
*/

static void prv_periodic_broadcast_tx(SoftTimerId timer_id, void *context) {
  MotorControllerBroadcastStorage *storage = context;
  if (storage->velocity_rx_bitset == (1 << NUM_MOTOR_CONTROLLERS) - 1) {
    // Received speed from all motor controllers - clear bitset and broadcast
    storage->velocity_rx_bitset = 0;
    prv_broadcast_speed(storage);
  }
  if (storage->bus_rx_bitset == (1 << NUM_MOTOR_CONTROLLERS) - 1) {
    // Received bus measurements from all motor controllers - clear bitset and broadcast
    storage->bus_rx_bitset = 0;
    prv_broadcast_bus_measurement(storage);
  }
  soft_timer_start_millis(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS, prv_periodic_broadcast_tx,
                          storage, NULL);
}

StatusCode mci_broadcast_init(MotorControllerBroadcastStorage *storage,
                              MotorControllerBroadcastSettings *settings) {
  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    storage->ids[motor_id] = settings->device_ids[motor_id];
  }
  storage->velocity_rx_bitset = 0;
  storage->bus_rx_bitset = 0;

  /*
  // Velocity Measurements
  status_ok_or_return(
      generic_can_register_rx(settings->motor_can, prv_handle_speed_rx, GENERIC_CAN_EMPTY_MASK,
                              MOTOR_CAN_LEFT_VELOCITY_MEASUREMENT_FRAME_ID, false, storage));

  status_ok_or_return(
      generic_can_register_rx(settings->motor_can, prv_handle_speed_rx, GENERIC_CAN_EMPTY_MASK,
                              MOTOR_CAN_RIGHT_VELOCITY_MEASUREMENT_FRAME_ID, false, storage));

  // Bus Mesurements
  status_ok_or_return(generic_can_register_rx(
      settings->motor_can, prv_handle_bus_measurement_rx, GENERIC_CAN_EMPTY_MASK,
      MOTOR_CAN_LEFT_BUS_MEASUREMENT_FRAME_ID, false, storage));

  status_ok_or_return(generic_can_register_rx(
      settings->motor_can, prv_handle_bus_measurement_rx, GENERIC_CAN_EMPTY_MASK,
      MOTOR_CAN_RIGHT_BUS_MEASUREMENT_FRAME_ID, false, storage));
  */

  return soft_timer_start_millis(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS, prv_periodic_broadcast_tx,
                                 storage, NULL);
}
