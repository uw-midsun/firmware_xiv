#pragma once

#include "motor_controller.h"
#include "status.h"
#include "motor_can.h"

typedef struct MotorDriveCommand {
    float motor_velocity;
    float motor_current;
} MotorDriveCommand;

void drive_output_enable(MotorControllerStorage *storage);

void drive_output_disable(MotorControllerStorage *storage);

StatusCode drive_output_init(MotorControllerStorage *controller);
