#pragma once

#include "generic_can.h"

#include "mci_broadcast.h"
#include "motor_can.h"
#include "wavesculptor.h"

#define MOTOR_TEMPERATURE_TX_PERIOD_MS 500

typedef struct MotorTemperatureMeasurements {
  WaveSculptorDspTempMeasurement dsp_measurements[NUM_MOTOR_CONTROLLERS];
  WaveSculptorSinkMotorTempMeasurement sink_motor_measurements[NUM_MOTOR_CONTROLLERS];
} MotorTemperatureMeasurements;

typedef struct MotorTemperatureStorage {
  MotorTemperatureMeasurements measurements;
  MotorCanDeviceId ids[NUM_MOTOR_CONTROLLERS];
} MotorTemperatureStorage;

typedef struct MotorTemperatureSettings {
  GenericCan *motor_can;
  MotorCanDeviceId device_ids[NUM_MOTOR_CONTROLLERS];
} MotorTemperatureSettings;

StatusCode motor_temperature_init(MotorTemperatureStorage *storage,
                                  MotorTemperatureSettings *setting);
