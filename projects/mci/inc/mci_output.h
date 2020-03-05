#pragma once

#include "generic_can.h"
#include "pedal_rx.h"

#define MOTOR_CONTROLLER_BRAKE_THRESHOLD 0.0f
#define MOTOR_CONTROLLER_PEDAL_MAX 100.0f

#define MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS 200

typedef struct {
  GenericCan *motor_can;
  PedalRxStorage pedal_storage;
} MotorControllerOutputStorage;

StatusCode mci_output_init(MotorControllerOutputStorage *storage, GenericCan *motor_can_settings);
