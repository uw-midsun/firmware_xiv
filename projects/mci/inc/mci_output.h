#pragma once

#include "motor_controller.h"

#define MOTOR_CONTROLLER_BRAKE_THRESHOLD 0.0f

#define MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS 200

StatusCode mci_output_init(MotorControllerStorage *storage);
