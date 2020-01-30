#include "mci_broadcast.h"

#include "motor_can.h"
#include "generic_can_msg.h"

#include "can_transmit.h"
#include "can_unpack.h"

static void prv_handle_speed(int16_t speed_cms[], size_t num_speeds, void *context) {
  MotorControllerStorage *storage = context;
  // left motor is motor of truth
  storage->motor_velocity = speed_cms[MOTOR_CONTROLLER_LEFT];

  CAN_TRANSMIT_MOTOR_VELOCITY((uint16_t)speed_cms[MOTOR_CONTROLLER_LEFT], (uint16_t)speed_cms[MOTOR_CONTROLLER_RIGHT]);
}

static void prv_handle_bus_measurement(MotorControllerBusMeasurement measurements[],
                                       size_t num_measurements, void *context) {
  // Output system can
  CAN_TRANSMIT_MOTOR_CONTROLLER_VC(
      (uint16_t)measurements[MOTOR_CONTROLLER_LEFT].bus_voltage, (uint16_t)measurements[MOTOR_CONTROLLER_LEFT].bus_current,
      (uint16_t)measurements[MOTOR_CONTROLLER_RIGHT].bus_voltage, (uint16_t)measurements[MOTOR_CONTROLLER_RIGHT].bus_current);
}

StatusCode mci_broadcast_init(MotorControllerStorage *controller) {
  // Velocity Measurements
  status_ok_or_return(generic_can_register_rx(
    controller->settings.motor_can, prv_handle_speed, GENERIC_CAN_EMPTY_MASK,
    MOTOR_CAN_LEFT_VELOCITY_MEASUREMENT_FRAME_ID, false, controller));
  
  status_ok_or_return(generic_can_register_rx(
    controller->settings.motor_can, prv_handle_speed, GENERIC_CAN_EMPTY_MASK,
    MOTOR_CAN_RIGHT_VELOCITY_MEASUREMENT_FRAME_ID, false, controller));

  // Bus Mesurements
  status_ok_or_return(generic_can_register_rx(
    controller->settings.motor_can, prv_handle_bus_measurement, GENERIC_CAN_EMPTY_MASK,
    MOTOR_CAN_LEFT_BUS_MEASUREMENT_FRAME_ID, false, controller));
  
  status_ok_or_return(generic_can_register_rx(
    controller->settings.motor_can, prv_handle_bus_measurement, GENERIC_CAN_EMPTY_MASK,
    MOTOR_CAN_RIGHT_BUS_MEASUREMENT_FRAME_ID, false, controller));

  return STATUS_CODE_OK;
}