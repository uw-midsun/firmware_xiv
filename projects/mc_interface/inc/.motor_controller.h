#pragma once
// Interfaces with WaveSculptor20 motor controllers
// Requires generic CAN and soft timers to be initialized.
//
// Expects a CAN UART slave
// Repeats the last received message every 50ms - seems to result in smoother transitions
//
// In cruise mode, the first motor controller is picked as a master and set to velocity control.
// Its current reading is copied over to all other motor controllers to allow them to handle turns.
//
// We also require setpoint updates within ~250ms or we disable the motors as a safety precaution.
#include "exported_enums.h"
#include "generic_can.h"

#define MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS 200
// Arbitrary timeout after 5 TX periods without receiving a setpoint update
#define MOTOR_CONTROLLER_WATCHDOG_COUNTER 5

// Called with an array of reported vehicle speeds in cm/s when a new set of information
// is received from all motor controllers.
typedef void (*MotorControllerSpeedCb)(int16_t speed_cms[], size_t num_speeds, void *context);

// Called with arrays of reported bus voltages (V) and currents (A) when a new set of information
// is received from all motor controllers.
typedef struct MotorControllerBusMeasurement {
  int16_t bus_voltage;  // V
  int16_t bus_current;  // A
} MotorControllerBusMeasurement;
typedef void (*MotorControllerBusMeasurementCb)(MotorControllerBusMeasurement measurements[],
                                                size_t num_measurements, void *context);

typedef enum {
  MOTOR_CONTROLLER_LEFT,
  MOTOR_CONTROLLER_RIGHT,
  NUM_MOTOR_CONTROLLERS
} MotorController;

typedef enum {
  MOTOR_CONTROLLER_MODE_TORQUE = 0,
  MOTOR_CONTROLLER_MODE_VELOCITY,
  NUM_MOTOR_CONTROLLER_MODES,
} MotorControllerMode;

typedef uint32_t MotorControllerCanId;

// TODO: Break this API once we switch over completely to DBC
typedef struct MotorControllerSettings {
  GenericCan *motor_can;
  struct {
    // WaveSculptor address
    MotorControllerCanId motor_controller;
    // Driver Controls address
    MotorControllerCanId interface;
  } ids[NUM_MOTOR_CONTROLLERS];
  // Maximum bus current (should be programmed into the motor controllers)
  // Used to copy current setpoint from primary cruise controller
  float max_bus_current;

  MotorControllerSpeedCb speed_cb;
  MotorControllerBusMeasurementCb bus_measurement_cb;
  void *context;
} MotorControllerSettings;

typedef struct MotorControllerStorage {
  MotorControllerSettings settings;
  // For cruise control
  float cruise_current_percentage;
  bool cruise_is_braking;

  // Stored to repeat
  float target_velocity_ms;
  float target_current_percentage;
  MotorControllerMode target_mode;

  int16_t speed_cms[NUM_MOTOR_CONTROLLERS];
  MotorControllerBusMeasurement bus_measurement[NUM_MOTOR_CONTROLLERS];
  uint8_t speed_rx_bitset;
  uint8_t bus_rx_bitset;

  size_t timeout_counter;
} MotorControllerStorage;

// TODO: Break this API once we switch over completely to DBC
// |settings.motor_can| should be initialized.
StatusCode motor_controller_init(MotorControllerStorage *controller,
                                 const MotorControllerSettings *settings);

// Override the callbacks that are called when information is received from the motor controllers
StatusCode motor_controller_set_update_cbs(MotorControllerStorage *controller,
                                           MotorControllerSpeedCb speed_cb,
                                           MotorControllerBusMeasurementCb bus_measurement_cb,
                                           void *context);

// Switch the motor controllers to throttle control
// |throttle| should be -EE_DRIVE_OUTPUT_DENOMINATOR to EE_DRIVE_OUTPUT_DENOMINATOR
// where |throttle| < 0: brake, |throttle| == 0: coast, |throttle| > 0: accel
StatusCode motor_controller_set_throttle(MotorControllerStorage *controller, int16_t throttle,
                                         EEDriveOutputDirection direction);

// Switch the motor controllers to cruise control
StatusCode motor_controller_set_cruise(MotorControllerStorage *controller, int16_t speed_cms);
