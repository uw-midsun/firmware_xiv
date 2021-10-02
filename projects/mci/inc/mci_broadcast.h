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
  MOTOR_CONTROLLER_BROADCAST_SINK_MOTOR_TEMP,
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

// Limit flags -- see WS22 user manual for more info
typedef enum {
  MCI_LIMIT_OUTPUT_VOLTAGE = 0,
  MCI_LIMIT_MOTOR_CURRENT,
  MCI_LIMIT_VELOCITY,
  MCI_LIMIT_BUS_CURRENT,
  MCI_LIMIT_VOLTAGE_UPPER,
  MCI_LIMIT_VOLTAGE_LOWER,
  MCI_LIMIT_TEMPERATURE,
  NUM_MCI_LIMITS,
} MciLimit;
static_assert(NUM_MCI_LIMITS <= sizeof(uint8_t) * 8, "MciLimit is too large to store in a uint8_t");

// Error flags -- see WS22 user manual for more info
typedef enum {
  MCI_ERROR_RESERVED0 = 0,  // First bit reserved
  MCI_ERROR_SW_OVERCURRENT,
  MCI_ERROR_BUS_OVERVOLTAGE,
  MCI_ERROR_BAD_POSITION,
  MCI_ERROR_WATCHDOG,
  MCI_ERROR_CONFIG_READ,
  MCI_ERROR_UVLO,
  MCI_ERROR_OVERSPEED,
  NUM_MCI_ERRORS,
} MciError;
static_assert(NUM_MCI_ERRORS <= sizeof(uint8_t) * 8, "MciError is too large to store in a uint8_t");

// To mask out reserved bits
#define MCI_LIMIT_MASK ((1 << NUM_MCI_LIMITS) - 1)
#define MCI_ERROR_MASK ((1 << NUM_MCI_ERRORS) - 1)

// Stores status info for broadcasting
typedef struct {
  uint8_t mc_limit_bitset[NUM_MOTOR_CONTROLLERS];  // from each WS
  uint8_t mc_error_bitset[NUM_MOTOR_CONTROLLERS];  // from each WS
  uint8_t board_fault_bitset;                      // from mci_fan_control
  uint8_t mc_overtemp_bitset;                      // from temp messages
} MciStatusMessage;
static_assert(sizeof(MciStatusMessage) <= sizeof(uint64_t),
              "MciStatusMessage is too large to store in a uint64_t");

typedef struct MotorControllerMeasurements {
  WaveSculptorBusMeasurement bus_measurements[NUM_MOTOR_CONTROLLERS];
  float vehicle_velocity[NUM_MOTOR_CONTROLLERS];
  MciStatusMessage status;
  WaveSculptorSinkMotorTempMeasurement sink_motor_measurements[NUM_MOTOR_CONTROLLERS];
  WaveSculptorDspTempMeasurement dsp_measurements[NUM_MOTOR_CONTROLLERS];
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
  uint8_t motor_sink_rx_bitset;
  uint8_t dsp_rx_bitset;
  MotorControllerMeasurements measurements;
  MotorCanDeviceId ids[NUM_MOTOR_CONTROLLERS];
  // What we're currently filtering for
  MotorController filter_mc;
  MotorControllerBroadcastMeasurement filter_measurement;
} MotorControllerBroadcastStorage;

StatusCode mci_broadcast_init(MotorControllerBroadcastStorage *storage,
                              MotorControllerBroadcastSettings *settings);
