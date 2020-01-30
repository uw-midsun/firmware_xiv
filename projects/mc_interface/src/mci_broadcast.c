#include "mci_broadcast.h"

#include "motor_can.h"
#include "generic_can_msg.h"

#include "can_transmit.h"
#include "can_unpack.h"
#include "critical_section.h"

static void prv_broadcast_speed(MotorControllerStorage *storage) {
  // left motor is motor of truth
  CAN_TRANSMIT_MOTOR_VELOCITY(
    (uint16_t)storage->vehicle_velocity[MOTOR_CONTROLLER_LEFT],
    (uint16_t)storage->vehicle_velocity[MOTOR_CONTROLLER_RIGHT]);
}

static void prv_broadcast_bus_measurement(MotorControllerStorage *storage) {
  // Output system can
  CAN_TRANSMIT_MOTOR_CONTROLLER_VC(
    (uint16_t)storage->bus_measurements[MOTOR_CONTROLLER_LEFT].bus_voltage_v,
    (uint16_t)storage->bus_measurements[MOTOR_CONTROLLER_LEFT].bus_current_a,
    (uint16_t)storage->bus_measurements[MOTOR_CONTROLLER_RIGHT].bus_voltage_v,
    (uint16_t)storage->bus_measurements[MOTOR_CONTROLLER_RIGHT].bus_current_a);
}

static void prv_handle_speed(const GenericCanMsg *msg, void *context) {
  MotorControllerStorage *storage = context;
  WaveSculptorCanId can_id = { .raw = msg->id };
  WaveSculptorCanData can_data = { .raw = msg->data };

  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    if (can_id.device_id == storage->settings.motor_controller_ids[i]) {
      storage->vehicle_velocity[i] = can_data.velocity_measurement.vehicle_velocity_ms * 100;
      storage->speed_rx_bitset |= 1 << i;
      break;
    }
  }
}

static void prv_handle_bus_measurement(const GenericCanMsg *msg, void *context) {
  bool disabled = critical_section_start();
  MotorControllerStorage *storage = context;

  WaveSculptorCanId can_id = { .raw = msg->id };
  WaveSculptorCanData can_data = { .raw = msg->data };

  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    if (can_id.device_id == storage->settings.motor_controller_ids[i]) {
      storage->bus_measurements[i] = can_data.bus_measurement;
      storage->bus_rx_bitset |= 1 << i;
    }
  }
  critical_section_end(disabled);
}

static void prv_periodic_broadcast_tx(SoftTimerId timer_id, void *context) {
  MotorControllerStorage *storage = context;
  if (storage->speed_rx_bitset == (1 << NUM_MOTOR_CONTROLLERS) - 1) {
    // Received speed from all motor controllers - clear bitset and broadcast
    storage->speed_rx_bitset = 0;
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

StatusCode mci_broadcast_init(MotorControllerStorage *storage) {
  // Velocity Measurements
  status_ok_or_return(generic_can_register_rx(
    storage->settings.motor_can, prv_handle_speed, GENERIC_CAN_EMPTY_MASK,
    MOTOR_CAN_LEFT_VELOCITY_MEASUREMENT_FRAME_ID, false, storage));

  status_ok_or_return(generic_can_register_rx(
    storage->settings.motor_can, prv_handle_speed, GENERIC_CAN_EMPTY_MASK,
    MOTOR_CAN_RIGHT_VELOCITY_MEASUREMENT_FRAME_ID, false, storage));

  // Bus Mesurements
  status_ok_or_return(generic_can_register_rx(
    storage->settings.motor_can, prv_handle_bus_measurement, GENERIC_CAN_EMPTY_MASK,
    MOTOR_CAN_LEFT_BUS_MEASUREMENT_FRAME_ID, false, storage));

  status_ok_or_return(generic_can_register_rx(
    storage->settings.motor_can, prv_handle_bus_measurement, GENERIC_CAN_EMPTY_MASK,
    MOTOR_CAN_RIGHT_BUS_MEASUREMENT_FRAME_ID, false, storage));
  return soft_timer_start_millis(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS,
                                 prv_periodic_broadcast_tx, storage, NULL);
}
