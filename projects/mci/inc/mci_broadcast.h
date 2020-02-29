#pragma once

#include "wavesculptor.h"

#define MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS 400

typedef enum {
  LEFT_MOTOR_CONTROLLER = 0,
  RIGHT_MOTOR_CONTROLLER,
  NUM_MOTOR_CONTROLLERS,
} MotorController;

typedef struct MotorControllerMeasurements {
  WaveSculptorBusMeasurement bus_measurements[NUM_MOTOR_CONTROLLERS];
  float vehicle_velocity[NUM_MOTOR_CONTROLLERS];
} MotorControllerMeasurements;

typedef struct MotorControllerBroadcastSettings {
  GenericCan *motor_can;
  MotorCanDeviceId device_ids[NUM_MOTOR_CONTROLLERS];
} MotorControllerBroadcastSettings;

typedef struct MotorControllerBroadcastStorage {
  uint8_t bus_rx_bitset;
  uint8_t velocity_rx_bitset;
  MotorControllerMeasurements measurements; 
  MotorCanDeviceId ids[NUM_MOTOR_CONTROLLERS];
} MotorControllerBroadcastStorage;

StatusCode mci_broadcast_init(MotorControllerBroadcastStorage *storage,
                              MotorControllerBroadcastSettings *settings);