#include "motor_controller.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

#include "can_transmit.h"
#include "critical_section.h"
#include "motor_can.h"
#include "soft_timer.h"
#include "wavesculptor.h"

#include "log.h"
#include "delay.h"

// Torque control mode:
// - velocity = +/-100 m/s
// - current = 0 to 100% (throttle position)
// Velocity control mode:
// - velocity = target m/s
// - current = 100%
// Regen braking:
// - velocity = 0
// - current = braking force

static void prv_bus_measurement_rx(const GenericCanMsg *msg, void *context) {
  bool disabled = critical_section_start();
  MotorControllerStorage *storage = context;
  WaveSculptorCanId can_id = { .raw = msg->id };
  WaveSculptorCanData can_data = { .raw = msg->data };

  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    if (can_id.device_id == storage->settings.ids[i].motor_controller) {
      storage->bus_measurement[i].bus_voltage = (int16_t)(can_data.bus_measurement.bus_voltage_v);
      storage->bus_measurement[i].bus_current = (int16_t)(can_data.bus_measurement.bus_current_a);
      storage->bus_rx_bitset |= 1 << i;

      if (i == 0) {
        // This controller is deemed to be the source of truth during cruise - copy setpoint
        storage->cruise_current_percentage =
            fabsf(can_data.bus_measurement.bus_current_a / storage->settings.max_bus_current);
        storage->cruise_is_braking = (can_data.bus_measurement.bus_current_a < 0.0f);
      }
    }
  }

  if (storage->bus_rx_bitset == (1 << NUM_MOTOR_CONTROLLERS) - 1) {
    // Received speed from all motor controllers - clear bitset and broadcast
    storage->bus_rx_bitset = 0;
    storage->settings.bus_measurement_cb(storage->bus_measurement, NUM_MOTOR_CONTROLLERS,
                                         storage->settings.context);
  }

  critical_section_end(disabled);

}

static void prv_velocity_measurement_rx(const GenericCanMsg *msg, void *context) {
  MotorControllerStorage *storage = context;
  WaveSculptorCanId can_id = { .raw = msg->id };
  WaveSculptorCanData can_data = { .raw = msg->data };

  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    if (can_id.device_id == storage->settings.ids[i].motor_controller) {
      storage->speed_cms[i] = (int16_t)(can_data.velocity_measurement.vehicle_velocity_ms * 100);
      storage->speed_rx_bitset |= 1 << i;
      break;
    }
  }

  if (storage->speed_rx_bitset == (1 << NUM_MOTOR_CONTROLLERS) - 1) {
    // Received speed from all motor controllers - clear bitset and broadcast
    storage->speed_rx_bitset = 0;
    storage->settings.speed_cb(storage->speed_cms, NUM_MOTOR_CONTROLLERS,
                               storage->settings.context);
  }
}

static void prv_periodic_tx(SoftTimerId timer_id, void *context) {
  MotorControllerStorage *storage = context;

  GenericCanMsg msg_left = {
    .id = MOTOR_CAN_LEFT_DRIVE_COMMAND_FRAME_ID,
    .dlc = 8u,
    .extended = false,
  };
  GenericCanMsg msg_right = {
    .id = MOTOR_CAN_RIGHT_DRIVE_COMMAND_FRAME_ID,
    .dlc = 8,
    .extended = false,
  };

  // We only use Drive Commands
  struct motor_can_left_drive_command_t left_drive_cmd = { 0 };
  struct motor_can_right_drive_command_t right_drive_cmd = { 0 };

  if (storage->timeout_counter > MOTOR_CONTROLLER_WATCHDOG_COUNTER) {
    // We haven't received an update from driver controls in a while. As
    // such, we'll do the safe thing and send coast commands.
    left_drive_cmd.motor_velocity = 0.0f;
    left_drive_cmd.motor_current = 0.0f;

    right_drive_cmd.motor_velocity = 0.0f;
    right_drive_cmd.motor_current = 0.0f;

    LOG_DEBUG("Motor watchdog\n"); 

  } else if (storage->target_mode == MOTOR_CONTROLLER_MODE_TORQUE) {
    // If we are in Torque Control mode, then we just send the same message
    // to both Motor Controllers
    left_drive_cmd.motor_velocity = storage->target_velocity_ms;
    left_drive_cmd.motor_current = storage->target_current_percentage;

    right_drive_cmd.motor_velocity = storage->target_velocity_ms;
    right_drive_cmd.motor_current = storage->target_current_percentage;
    LOG_DEBUG("Motor torque mode\n"); 

  } else {
    // For Cruise Control mode, we allow the Motor Controller's control loop
    // to perform the calculations. In order to do so, WLOG we designate the
    // Left Motor Controller as the primary, and copy the current setpoint
    // value (received via the Telemetry messages from the Primary) and use
    // that as the current setpoint for the second controller.
    left_drive_cmd.motor_velocity = storage->target_velocity_ms;
    left_drive_cmd.motor_current = storage->target_current_percentage;

    right_drive_cmd.motor_velocity =
        (storage->cruise_is_braking) ? 0.0f : WAVESCULPTOR_FORWARD_VELOCITY;
    right_drive_cmd.motor_current = storage->cruise_current_percentage;
    LOG_DEBUG("cruise control mode\n"); 

  }

  LOG_DEBUG("LEFT: %i %i\n", (int) (left_drive_cmd.motor_velocity*100), (int)(left_drive_cmd.motor_current*100));
  LOG_DEBUG("RIGHT: %i %i\n", (int) (right_drive_cmd.motor_velocity*100), (int)(right_drive_cmd.motor_current*100));
 
 


  uint8_t data_left[MOTOR_CAN_LEFT_DRIVE_COMMAND_LENGTH] = { 0 };
  uint8_t data_right[MOTOR_CAN_RIGHT_DRIVE_COMMAND_LENGTH] = { 0 };
  motor_can_left_drive_command_pack(data_left, &left_drive_cmd, sizeof(data_left));
  motor_can_right_drive_command_pack(data_right, &right_drive_cmd, sizeof(data_right));

  memcpy(&msg_left.data, data_left, sizeof(data_left));
  memcpy(&msg_right.data, data_right, sizeof(data_right));

  generic_can_tx(storage->settings.motor_can, &msg_left);
  generic_can_tx(storage->settings.motor_can, &msg_right);

  // Increment watchdog counter
  storage->timeout_counter++;

  soft_timer_start_millis(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS, prv_periodic_tx, storage, NULL);
}

StatusCode motor_controller_init(MotorControllerStorage *controller,
                                 const MotorControllerSettings *settings) {
  memset(controller, 0, sizeof(*controller));
  controller->settings = *settings;

  WaveSculptorCanId can_id = { 0 };

  // Velocity Measurements
  status_ok_or_return(generic_can_register_rx(
      controller->settings.motor_can, prv_velocity_measurement_rx, GENERIC_CAN_EMPTY_MASK,
      MOTOR_CAN_LEFT_VELOCITY_MEASUREMENT_FRAME_ID, false, controller));
  status_ok_or_return(generic_can_register_rx(
      controller->settings.motor_can, prv_velocity_measurement_rx, GENERIC_CAN_EMPTY_MASK,
      MOTOR_CAN_RIGHT_VELOCITY_MEASUREMENT_FRAME_ID, false, controller));

  // Bus Mesurements
  status_ok_or_return(generic_can_register_rx(
      controller->settings.motor_can, prv_bus_measurement_rx, GENERIC_CAN_EMPTY_MASK,
      MOTOR_CAN_LEFT_BUS_MEASUREMENT_FRAME_ID, false, controller));
  status_ok_or_return(generic_can_register_rx(
      controller->settings.motor_can, prv_bus_measurement_rx, GENERIC_CAN_EMPTY_MASK,
      MOTOR_CAN_RIGHT_BUS_MEASUREMENT_FRAME_ID, false, controller));

  return soft_timer_start_millis(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS, prv_periodic_tx, controller,
                                 NULL);
}

// Override the callbacks that are called when information is received from the motor controllers
StatusCode motor_controller_set_update_cbs(MotorControllerStorage *controller,
                                           MotorControllerSpeedCb speed_cb,
                                           MotorControllerBusMeasurementCb bus_measurement_cb,
                                           void *context) {
  bool disabled = critical_section_start();
  controller->settings.speed_cb = speed_cb;
  controller->settings.bus_measurement_cb = bus_measurement_cb;
  controller->settings.context = context;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode motor_controller_set_throttle(MotorControllerStorage *controller, int16_t throttle,
                                         EEDriveOutputDirection direction) {
  // Use impossible target velocity for torque control
  const float velocity_lookup[] = {
    [EE_DRIVE_OUTPUT_DIRECTION_NEUTRAL] = 0.0f,
    [EE_DRIVE_OUTPUT_DIRECTION_FORWARD] = WAVESCULPTOR_FORWARD_VELOCITY,
    [EE_DRIVE_OUTPUT_DIRECTION_REVERSE] = WAVESCULPTOR_REVERSE_VELOCITY,
  };

  float target_velocity = velocity_lookup[direction];

  if (direction == EE_DRIVE_OUTPUT_DIRECTION_NEUTRAL) {
    // Neutral - coast
    throttle = 0;
  } else if (throttle < 0) {
    // Braking - velocity = 0, brake %
    throttle *= -1;
    target_velocity = 0.0f;
  }

  // Reset counter
  bool disabled = critical_section_start();
  controller->timeout_counter = 0;
  controller->target_velocity_ms = target_velocity,
  controller->target_current_percentage = (float)throttle / EE_DRIVE_OUTPUT_DENOMINATOR;
  controller->target_mode = MOTOR_CONTROLLER_MODE_TORQUE;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode motor_controller_set_cruise(MotorControllerStorage *controller, int16_t speed_cms) {
  // Cruise control is always forward

  bool disabled = critical_section_start();
  controller->timeout_counter = 0;
  controller->target_velocity_ms = (float)speed_cms / 100;
  controller->target_current_percentage = 1.0f;

  // Start with no information on cruise state
  controller->cruise_current_percentage = 0.0f;
  controller->cruise_is_braking = false;
  controller->target_mode = MOTOR_CONTROLLER_MODE_VELOCITY;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}
