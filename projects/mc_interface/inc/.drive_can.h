#pragma once
// Handles drive output messages from Driver Controls
// Requires CAN and motor controllers to be initialized
//
// Raises events accordingly
#include "motor_controller.h"
#include "status.h"

StatusCode drive_can_init(MotorControllerStorage *controller);
