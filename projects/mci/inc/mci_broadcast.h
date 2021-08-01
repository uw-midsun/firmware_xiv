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

typedef struct MotorControllerMeasurements {
  WaveSculptorBusMeasurement bus_measurements[NUM_MOTOR_CONTROLLERS];
  WaveSculptorSinkMotorTempMeasurement temp_measurements[NUM_MOTOR_CONTROLLERS];
  WaveSculptorDspTempMeasurement dsp_measurements[NUM_MOTOR_CONTROLLERS];
  float vehicle_velocity[NUM_MOTOR_CONTROLLERS];
  uint32_t status[NUM_MOTOR_CONTROLLERS];
} MotorControllerMeasurements;

typedef struct MotorControllerBroadcastSettings {
  Mcp2515Storage *motor_can;
  MotorCanDeviceId device_ids[NUM_MOTOR_CONTROLLERS];
} MotorControllerBroadcastSettings;

// Kinda in a weird situation with the heat sink and motor temperature.
// The WaveSculptor bundles the sink and motor temperatures as one but separates DSP temperature
// Maybe one way we can do it is similar to the bus measurements with the voltage and current
typedef struct MotorControllerBroadcastStorage {
  Mcp2515Storage *motor_can;
  uint8_t bus_rx_bitset;
  uint8_t velocity_rx_bitset;
  uint8_t status_rx_bitset;
  uint8_t sink_rx_bitset;
  uint8_t temp_rx_bitset;
  uint8_t dsp_rx_bitset;
  MotorControllerMeasurements measurements;
  MotorCanDeviceId ids[NUM_MOTOR_CONTROLLERS];
  // What we're currently filtering for
  MotorController filter_mc;
  MotorControllerBroadcastMeasurement filter_measurement;
} MotorControllerBroadcastStorage;

StatusCode mci_broadcast_init(MotorControllerBroadcastStorage *storage,
                              MotorControllerBroadcastSettings *settings);
