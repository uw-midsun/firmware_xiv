#pragma once

#include "generic_can.h"

#include "motor_can.h"
#include "wavesculptor.h"

#include "mcp2515.h"

#define MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS 400

typedef void (*MotorControllerMeasurementCallback)(const GenericCanMsg *msg, void *context);

// Measurements from the WaveSculptor we process currently
typedef enum {
  MOTOR_CONTROLLER_BROADCAST_STATUS = 0,
  MOTOR_CONTROLLER_BROADCAST_BUS,
  MOTOR_CONTROLLER_BROADCAST_VELOCITY,
  MOTOR_CONTROLLER_BROADCAST_MOTOR_TEMP,
  MOTOR_CONTROLLER_BROADCAST_DSP_TEMP,
  NUM_MOTOR_CONTROLLER_BROADCAST_MEASUREMENTS,
} MotorControllerBroadcastMeasurement;

typedef enum {
  LEFT_MOTOR_CONTROLLER = 0,
  RIGHT_MOTOR_CONTROLLER,
  NUM_MOTOR_CONTROLLERS,
} MotorController;

// NOTE(Feb 2021 validation): currently both motor controllers are 0x400, right will need to be
// configured
#define LEFT_MOTOR_CONTROLLER_BASE_ADDR 0x400
#define RIGHT_MOTOR_CONTROLLER_BASE_ADDR 0x200

#define MOTOR_CONTROLLER_BASE_ADDR_LOOKUP(controller)                        \
  (((controller) == LEFT_MOTOR_CONTROLLER) ? LEFT_MOTOR_CONTROLLER_BASE_ADDR \
                                           : RIGHT_MOTOR_CONTROLLER_BASE_ADDR)

// Stores callbacks when processing MCP2515 messages
typedef struct {
  MotorController motor_controller;
  MotorControllerBroadcastMeasurement cur_measurement;
  MotorControllerMeasurementCallback callbacks[NUM_MOTOR_CONTROLLER_BROADCAST_MEASUREMENTS];
} MotorControllerCallbackStorage;

typedef struct MotorControllerMeasurements {
  WaveSculptorBusMeasurement bus_measurements[NUM_MOTOR_CONTROLLERS];
  float vehicle_velocity[NUM_MOTOR_CONTROLLERS];
  uint32_t status[NUM_MOTOR_CONTROLLERS];
} MotorControllerMeasurements;

typedef struct MotorControllerBroadcastSettings {
  Mcp2515Storage *motor_can;
  MotorCanDeviceId device_ids[NUM_MOTOR_CONTROLLERS];
} MotorControllerBroadcastSettings;

typedef struct MotorControllerBroadcastStorage {
  Mcp2515Storage *motor_can;
  uint8_t bus_rx_bitset;
  uint8_t velocity_rx_bitset;
  uint8_t status_rx_bitset;
  MotorControllerMeasurements measurements;
  MotorCanDeviceId ids[NUM_MOTOR_CONTROLLERS];
  // Callbacks exposed for unit testing
  MotorControllerCallbackStorage cb_storage;
} MotorControllerBroadcastStorage;

StatusCode mci_broadcast_init(MotorControllerBroadcastStorage *storage,
                              MotorControllerBroadcastSettings *settings);
