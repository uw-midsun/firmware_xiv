#pragma once

#include "generic_can.h"
#include "pedal_rx.h"
#include "mcp2515.h"

#define MOTOR_CONTROLLER_BRAKE_THRESHOLD 0.0f
#define MOTOR_CONTROLLER_PEDAL_MAX 100.0f

#define MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS 200

#define MCI_PEDAL_RX_TIMEOUT_MS 250

typedef struct {
  Mcp2515Storage *motor_can;
  PedalRxStorage pedal_storage;
} MotorControllerOutputStorage;

StatusCode mci_output_init(MotorControllerOutputStorage *storage, Mcp2515Storage *motor_can);
