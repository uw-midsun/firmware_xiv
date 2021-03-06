#pragma once
// WaveSculptor 22 CAN definitions
// Refer to the user manual for more info
#include <assert.h>
#include <stdint.h>

// Set to unobtainable values to control based off current
// (see pg. 30 of WaveSculptor user manual)
#define WAVESCULPTOR_FORWARD_VELOCITY 20000.0f
#define WAVESCULPTOR_REVERSE_VELOCITY -20000.0f

typedef union WaveSculptorCanId {
  struct {
    uint16_t msg_id : 5;
    uint16_t device_id : 6;
  };
  uint16_t raw;
} WaveSculptorCanId;

// Send to WaveSculptor (Drive Commands)
typedef enum {
  WAVESCULPTOR_CMD_ID_DRIVE = 0x01,
  WAVESCULPTOR_CMD_ID_POWER = 0x02,
  WAVESCULPTOR_CMD_ID_RESET = 0x03,
} WaveSculptorCmdId;

// Driver Controls Base Addr + 0x01
typedef struct WaveSculptorDriveCmd {
  float motor_velocity_rpm;
  float motor_current_percentage;
} WaveSculptorDriveCmd;
static_assert(sizeof(WaveSculptorDriveCmd) == 8, "WaveSculptorDriveCmd is not 8 bytes");

// Driver Controls Base Addr + 0x02
typedef struct WaveSculptorPowerCmd {
  uint32_t reserved;
  float bus_current_percentage;
} WaveSculptorPowerCmd;
static_assert(sizeof(WaveSculptorPowerCmd) == 8, "WaveSculptorPowerCmd is not 8 bytes");

// Driver Controls Base Addr + 0x03
typedef struct WaveSculptorResetCmd {
  uint64_t reserved;
} WaveSculptorResetCmd;
static_assert(sizeof(WaveSculptorResetCmd) == 8, "WaveSculptorResetCmd is not 8 bytes");

// Send to WaveSculptor (Configuration Commands)
typedef enum {
  WAVESCULPTOR_CONFIG_ID_ACTIVE_MOTOR = 0x12,
} WaveSculptorConfigId;

// Motor Controller Base Addr + 0x12
typedef struct WaveSculptorActiveMotorChange {
  // Must spell "ACTMOT" in ASCII (0x54 4F 4D 54 43 41)
  uint16_t access_key[3];

  // Desired active motor (0 to 9)
  uint16_t active_motor;
} WaveSculptorActiveMotorChange;
static_assert(sizeof(WaveSculptorActiveMotorChange) == 8,
              "WaveSculptorActiveMotorChange is not 8 bytes");

// Receive from WaveSculptor
typedef enum {
  WAVESCULPTOR_MEASUREMENT_ID_INFO = 0x00,
  WAVESCULPTOR_MEASUREMENT_ID_STATUS = 0x01,
  WAVESCULPTOR_MEASUREMENT_ID_BUS = 0x02,
  WAVESCULPTOR_MEASUREMENT_ID_VELOCITY = 0x03,
  WAVESCULPTOR_MEASUREMENT_ID_PHASE_CURRENT = 0x04,
  WAVESCULPTOR_MEASUREMENT_ID_MOTOR_VOLTAGE_VECTOR = 0x05,
  WAVESCULPTOR_MEASUREMENT_ID_MOTOR_CURRENT_VECTOR = 0x06,
  WAVESCULPTOR_MEASUREMENT_ID_MOTOR_BACKEMF = 0x07,
  WAVESCULPTOR_MEASUREMENT_ID_POWER_RAIL = 0x08,
  WAVESCULPTOR_MEASUREMENT_ID_CONTROLLER_RAIL = 0x09,
  // ID 0x0A reserved
  WAVESCULPTOR_MEASUREMENT_ID_SINK_MOTOR_TEMPERATURE = 0x0B,
  WAVESCULPTOR_MEASUREMENT_ID_DSP_BOARD_TEMPERATURE = 0x0C,
  // ID 0x0D reserved
  WAVESCULPTOR_MEASUREMENT_ID_ODOMETER_BUS_AMPHOURS = 0x0E,
  WAVESCULPTOR_MEASUREMENT_ID_SLIP_SPEED = 0x17,
} WaveSculptorMeasurementId;

// Motor Controller Base Addr + 0x00
typedef struct WaveSculptorIdInfo {
  // Device identifier. 0x00004003
  uint32_t tritium_id;

  // Device serial number, allocated at manufacture.
  uint32_t serial_number;
} WaveSculptorIdInfo;
static_assert(sizeof(WaveSculptorIdInfo) == 8, "WaveSculptorIdInfo is not 8 bytes");

// Motor Controller Base Addr + 0x01
typedef struct WaveSculptorStatusInfo {
  // Limit flags indicate which control loop is limiting the output current of
  // the motor controller
  union {
    struct {
      uint16_t bridge_pwm : 1;
      uint16_t motor_current : 1;
      uint16_t velocity : 1;
      uint16_t bus_current : 1;
      uint16_t bus_voltage_upper : 1;
      uint16_t bus_voltage_lower : 1;
      uint16_t temperature : 1;
      uint16_t reserved : 9;
    } _PACKED;
    uint16_t raw;
  } limit_flags;

  // Error flags indicate errors
  union {
    struct {
      uint16_t reserved_0 : 1;
      uint16_t sw_overcurrent : 1;
      uint16_t dc_bus_overvoltage : 1;
      uint16_t motor_position : 1;
      uint16_t watchdog_reset : 1;
      uint16_t config_read_error : 1;
      uint16_t undervoltage_15v : 1;
      uint16_t desaturation : 1;
      uint16_t overspeed : 1;
      uint16_t reserved_1 : 7;
    } _PACKED;
    uint16_t raw;
  } error_flags;

  // The index of the motor currently being used.
  uint16_t active_motor;

  // The DSP CAN transmission error counter (CAN 2.0)
  uint8_t tx_err_cnt;

  // The DSP CAN receive error counter (CAN 2.0)
  uint8_t rx_err_cnt;
} WaveSculptorStatusInfo;
static_assert(sizeof(WaveSculptorStatusInfo) == 8, "WaveSculptorStatusInfo is not 8 bytes");

// Motor Controller Base Addr + 0x02
typedef struct WaveSculptorBusMeasurement {
  // Units: V
  // DC bus voltage at the controller.
  float bus_voltage_v;

  // Units: A
  // Current drawn from the DC bus by the controller.
  float bus_current_a;
} WaveSculptorBusMeasurement;
static_assert(sizeof(WaveSculptorBusMeasurement) == 8, "WaveSculptorBusMeasurement is not 8 bytes");

// Motor Controller Base Addr + 0x03
typedef struct WaveSculptorVelocityMeasurement {
  // Units: revolutions per minute (rpm)
  // Motor angular frequency.
  float motor_velocity_rpm;

  // Units: m/s
  // Vehicle velocity.
  float vehicle_velocity_ms;
} WaveSculptorVelocityMeasurement;
static_assert(sizeof(WaveSculptorVelocityMeasurement) == 8,
              "WaveSculptorVelocityMeasurement is not 8 bytes");

// Motor Controller Base Addr + 0x04
typedef struct WaveSculptorPhaseCurrentMeasurement {
  // Units: A
  // Root Mean Square current in motor Phase B.
  float phase_b_rms_current_a;

  // Units: A
  // Root Mean Square current in motor Phase C.
  float phase_c_rms_current_a;
} WaveSculptorPhaseCurrentMeasurement;
static_assert(sizeof(WaveSculptorPhaseCurrentMeasurement) == 8,
              "WaveSculptorPhaseCurrentMeasurement is not 8 bytes");

// Motor Controller Base Addr + 0x05
typedef struct WaveSculptorMotorVoltageVectorMeasurement {
  // Units: V
  // Imaginary component of the applied non-rotating voltage vector to the
  // motor.
  float voltage_imaginary_v;

  // Units: V
  // Real component of the applied non-rotating voltage vector to the motor.
  float voltage_real_v;
} WaveSculptorMotorVoltageVectorMeasurement;
static_assert(sizeof(WaveSculptorMotorVoltageVectorMeasurement) == 8,
              "WaveSculptorMotorVoltageVectorMeasurement is not 8 bytes");

// Motor Controller Base Addr + 0x06
typedef struct WaveSculptorMotorCurrentVectorMeasurement {
  // Units: A
  // Imaginary component of applied non-rotating current vector to the motor.
  // This current produces torque in the motor and should be in phase with the
  // back-EMF (electromotive force) of the motor.
  float current_imaginary_a;

  // Units: A
  // Real component of applied non-rotating current vector to the motor. This
  // vector represents the field current of the motor.
  float current_real_a;
} WaveSculptorMotorCurrentVectorMeasurement;
static_assert(sizeof(WaveSculptorMotorCurrentVectorMeasurement) == 8,
              "WaveSculptorMotorCurrentVectorMeasurement is not 8 bytes");

// Motor Controller Base Addr + 0x07
typedef struct WaveSculptorBackEmfMeasurement {
  // Units: V
  // Peak of the phase to neutral motor voltage.
  float back_emf_imaginary_v;

  // Units: V
  // By definition this value is always 0 V.
  float back_emf_real_v;
} WaveSculptorBackEmfMeasurement;
static_assert(sizeof(WaveSculptorBackEmfMeasurement) == 8,
              "WaveSculptorBackEmfMeasurement is not 8 bytes");

// Motor Controller Base Addr + 0x08
typedef struct WaveSculptorPowerRailMeasurement {
  float reserved;

  // Units: V
  // Actual voltage level of the 15 V power rail.
  float power_rail_v;
} WaveSculptorPowerRailMeasurement;
static_assert(sizeof(WaveSculptorPowerRailMeasurement) == 8,
              "WaveSculptorPowerRailMeasurement is not 8 bytes");

// Motor Controller Base Addr + 0x09
typedef struct WaveSculptorControllerRailMeasurement {
  // Units: V
  // Actual voltage level of the 1.9V DSP power rail.
  float power_rail_dsp_v;

  // Units: V
  // Actual voltage level of the 3.3V power rail.
  float power_rail_3_3_v;
} WaveSculptorControllerRailMeasurement;
static_assert(sizeof(WaveSculptorControllerRailMeasurement) == 8,
              "WaveSculptorControllerRailMeasurement is not 8 bytes");

// Motor Controller Base Addr + 0x0B
typedef struct WaveSculptorSinkMotorTempMeasurement {
  // Units: C
  // Internal temperature of the motor.
  float motor_temp_c;

  // Units: C
  // Internal temperature of the heatsink.
  float heatsink_temp_c;
} WaveSculptorSinkMotorTempMeasurement;
static_assert(sizeof(WaveSculptorSinkMotorTempMeasurement) == 8,
              "WaveSculptorSinkMotorTempMeasurement is not 8 bytes");

// Motor Controller Base Addr + 0x0C
typedef struct WaveSculptorDspTempMeasurement {
  // Units: C
  // Temperature of the DSP board.
  float dsp_temp_c;

  float reserved;
} WaveSculptorDspTempMeasurement;
static_assert(sizeof(WaveSculptorDspTempMeasurement) == 8,
              "WaveSculptorDspTempMeasurement is not 8 bytes");

// Motor Controller Base Addr + 0x0E
typedef struct WaveSculptorOdometerBusAhMeasurement {
  // Units: m
  // The distance the vehicle has travelled since reset.
  float odometer_m;

  // Units: Ah
  // The charge flow into the controller bus voltage from the time of reset.
  float dc_bus_ah;
} WaveSculptorOdometerBusAhMeasurement;
static_assert(sizeof(WaveSculptorOdometerBusAhMeasurement) == 8,
              "WaveSculptorOdometerBusAhMeasurement is not 8 bytes");

// Motor Controller Base Addr + 0x17
typedef struct WaveSculptorSlipSpeedMeasurement {
  float reserved;

  // Units: Hz
  // Slip speed when driving an induction motor.
  float slip_speed_hz;
} WaveSculptorSlipSpeedMeasurement;
static_assert(sizeof(WaveSculptorSlipSpeedMeasurement) == 8,
              "WaveSculptorSlipSpeedMeasurement is not 8 bytes");

typedef union WaveSculptorCanData {
  uint64_t raw;
  WaveSculptorDriveCmd drive_cmd;
  WaveSculptorPowerCmd power_cmd;
  WaveSculptorResetCmd reset_cmd;

  WaveSculptorActiveMotorChange active_motor_config;

  WaveSculptorIdInfo id_info;
  WaveSculptorStatusInfo status_info;
  WaveSculptorBusMeasurement bus_measurement;
  WaveSculptorVelocityMeasurement velocity_measurement;
  WaveSculptorPhaseCurrentMeasurement phase_current_measurement;
  WaveSculptorMotorVoltageVectorMeasurement motor_voltage_vector_measurement;
  WaveSculptorMotorCurrentVectorMeasurement motor_current_vector_measurement;
  WaveSculptorBackEmfMeasurement back_emf_measurement;
  WaveSculptorPowerRailMeasurement power_rail_measurement;
  WaveSculptorControllerRailMeasurement controller_rail_measurement;
  WaveSculptorSinkMotorTempMeasurement sink_motor_temp_measurement;
  WaveSculptorDspTempMeasurement dsp_temp_measurement;
  WaveSculptorOdometerBusAhMeasurement odometer_bus_ah_measurement;
  WaveSculptorSlipSpeedMeasurement slip_speed_measurement;
} WaveSculptorCanData;
