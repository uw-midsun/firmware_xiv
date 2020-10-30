#pragma once

#include "generic_can.h"

#include "motor_can.h"
#include "wavesculptor.h"

#define MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS 400

typedef void (*MotorControllerMeasurementCallback)(const GenericCanMsg *msg, void *context);

// This is replicated to some degree in motor_can.h, just using here for testing stuff
// using 6 IDs to replicate how we'd take in 6 IDs across two WaveSculptors, 
// but this should only have the first 3 when actually implemented 
typedef enum {
  MCI_BROADCAST_STATUS = 0, 
  MCI_BROADCAST_BUS, 
  MCI_BROADCAST_VELOCITY,
  MCI_BROADCAST_MOTOR_TEMP,
  MCI_BROADCAST_DSP_TEMP,
  NUM_MCI_BROADCAST_MEASUREMENTS,
} MciBroadcastMeasurementType;

// Message offsets from motor controller base address
#define MCI_BROADCAST_STATUS_OFFSET 0x1
#define MCI_BROADCAST_BUS_OFFSET 0x2
#define MCI_BROADCAST_VELOCITY_OFFSET 0x3
#define MCI_BROADCAST_MOTOR_TEMP_OFFSET 0xB
#define MCI_BROADCAST_DSP_TEMP_OFFSET 0xC

typedef enum {
  LEFT_MOTOR_CONTROLLER = 0,
  RIGHT_MOTOR_CONTROLLER,
  NUM_MOTOR_CONTROLLERS,
} MotorController;

#define LEFT_MOTOR_CONTROLLER_BASE_ADDR 0x400
#define RIGHT_MOTOR_CONTROLLER_BASE_ADDR 0x200 // idk what this actually should be 

#define MCI_ID_UNUSED 0x1 

#define MOTOR_CONTROLLER_BASE_ADDR_LOOKUP(controller) (((controller) == LEFT_MOTOR_CONTROLLER) ? LEFT_MOTOR_CONTROLLER_BASE_ADDR : RIGHT_MOTOR_CONTROLLER_BASE_ADDR)

// Stores callbacks when processing MCP2515 messages
typedef struct {
  MotorController motor_controller;
  MciBroadcastMeasurementType cur_measurement;
  MotorControllerMeasurementCallback callbacks[NUM_MCI_BROADCAST_MEASUREMENTS];
} MotorControllerCallbackStorage;

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
  MotorControllerMeasurementCallback callbacks[NUM_MCI_BROADCAST_MEASUREMENTS]; // Callbacks exposed so we can unit test more easily
} MotorControllerBroadcastStorage;

StatusCode mci_broadcast_init(MotorControllerBroadcastStorage *storage,
                              MotorControllerBroadcastSettings *settings);
