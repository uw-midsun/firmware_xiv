#pragma once
// WaveSculptor 20 CAN definitions
#include <assert.h>

#define WAVESCULPTOR_FORWARD_VELOCITY 100.0f
#define WAVESCULPTOR_REVERSE_VELOCITY -100.0f

typedef union WaveSculptorCanId {
  struct {
    uint16_t msg_id : 5;
    uint16_t device_id : 6;
  };
  uint16_t raw;
} WaveSculptorCanId;

// Send to WaveSculptor
typedef enum {
  WAVESCULPTOR_CMD_ID_DRIVE = 1,
  WAVESCULPTOR_CMD_ID_POWER = 2,
  WAVESCULPTOR_CMD_ID_RESET = 3,
} WaveSculptorCmdId;

// Driver Controls Base Addr + 1
typedef struct WaveSculptorDriveCmd {
  float motor_velocity_ms;
  float motor_current_percentage;
} WaveSculptorDriveCmd;

// Driver Controls Base Addr + 2
typedef struct WaveSculptorPowerCmd {
  uint32_t reserved;
  float bus_current_percentage;
} WaveSculptorPowerCmd;

// Driver Controls Base Addr + 3
typedef struct WaveSculptorResetCmd {
  uint64_t reserved;
} WaveSculptorResetCmd;

// Receive from WaveSculptor

typedef enum {
  WAVESCULPTOR_MEASUREMENT_ID_INFO = 0,
  WAVESCULPTOR_MEASUREMENT_ID_STATUS = 1,
  WAVESCULPTOR_MEASUREMENT_ID_BUS = 2,
  WAVESCULPTOR_MEASUREMENT_ID_VELOCITY = 3,
  WAVESCULPTOR_MEASUREMENT_ID_PHASE_CURRENT = 4,
  WAVESCULPTOR_MEASUREMENT_ID_MOTOR_VOLTAGE_VECTOR = 5,
  WAVESCULPTOR_MEASUREMENT_ID_MOTOR_CURRENT_VECTOR = 6,
  WAVESCULPTOR_MEASUREMENT_ID_MOTOR_BACKEMF = 7,
  WAVESCULPTOR_MEASUREMENT_ID_POWER_RAIL = 8,
  WAVESCULPTOR_MEASUREMENT_ID_CONTROLLER_RAIL = 9,
  WAVESCULPTOR_MEASUREMENT_ID_FAN_SPEED = 10,
  WAVESCULPTOR_MEASUREMENT_ID_SINK_MOTOR_TEMPERATURE = 11,
  WAVESCULPTOR_MEASUREMENT_ID_AIR_IN_CPU_TEMPERATURE = 12,
  WAVESCULPTOR_MEASUREMENT_ID_AIR_OUT_CAP_TEMPERATURE = 13,
  WAVESCULPTOR_MEASUREMENT_ID_ODOMETER_BUS_AMPHOURS = 14
} WaveSculptorMeasurementId;

// Motor Controller Base Addr + 0
typedef struct WaveSculptorIdInfo {
  // "TRIa" stored as a string. msg[0] = 'T', msg[1] = 'R'..
  char tritium_id[4];

  // Device serial number, allocated at manufacture.
  uint32_t serial_number;
} WaveSculptorIdInfo;
static_assert(sizeof(WaveSculptorIdInfo) == 8, "WaveSculptorIdInfo is not 8 bytes");

// Motor Controller Base Addr + 1
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
      uint16_t heatsink_temp : 1;
      uint16_t reserved : 9;
    } _PACKED;
    uint16_t raw;
  } limit_flags;

  // Error flags indicate errors
  union {
    struct {
      uint16_t hw_overcurrent : 1;
      uint16_t sw_overcurrent : 1;
      uint16_t dc_bus_overvoltage : 1;
      uint16_t motor_position : 1;
      uint16_t watchdog_reset : 1;
      uint16_t config_read_error : 1;
      uint16_t undervoltage_15v : 1;
      uint16_t reserved : 9;
    } _PACKED;
    uint16_t raw;
  } error_flags;

  // The index of the motor currently being used.
  uint16_t active_motor;
  uint16_t reserved;
} WaveSculptorStatusInfo;
static_assert(sizeof(WaveSculptorStatusInfo) == 8, "WaveSculptorStatusInfo is not 8 bytes");

// Motor Controller Base Addr + 2
typedef struct WaveSculptorBusMeasurement {
  // Units: V
  // DC bus voltage at the controller.
  float bus_voltage_v;

  // Units: A
  // Current drawn from the DC bus by the controller.
  float bus_current_a;
} WaveSculptorBusMeasurement;
static_assert(sizeof(WaveSculptorBusMeasurement) == 8, "WaveSculptorBusMeasurement is not 8 bytes");

// Motor Controller Base Addr + 3
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

// Motor Controller Base Addr + 4
typedef struct WaveSculptorPhaseCurrentMeasurement {
  // Units: A
  // Root Mean Square current in motor Phase B.
  float phase_b_rms_current_a;

  // Units: A
  // Root Mean Square current in motor Phase A.
  float phase_a_rms_current_a;
} WaveSculptorPhaseCurrentMeasurement;
static_assert(sizeof(WaveSculptorPhaseCurrentMeasurement) == 8,
              "WaveSculptorPhaseCurrentMeasurement is not 8 bytes");

// Motor Controller Base Addr + 5
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

// Motor Controller Base Addr + 6
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

// Motor Controller Base Addr + 7
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

// Motor Controller Base Addr + 8
typedef struct WaveSculptorPowerRailMeasurement {
  // Units: V
  // Actual voltage level of the 1.65 V analog reference.
  float power_rail_analog_ref_v;

  // Units: V
  // Actual voltage level of the 15 V power rail.
  float power_rail_v;
} WaveSculptorPowerRailMeasurement;
static_assert(sizeof(WaveSculptorPowerRailMeasurement) == 8,
              "WaveSculptorPowerRailMeasurement is not 8 bytes");

// Motor Controller Base Addr + 9
typedef struct WaveSculptorControllerRailMeasurement {
  // Units: V
  // Actual voltage level of the 1.2V DSP power rail.
  float power_rail_dsp_v;

  // Units: V
  // Actual voltage level of the 2.5V FPGA power rail.
  float power_rail_fgpa_v;
} WaveSculptorControllerRailMeasurement;
static_assert(sizeof(WaveSculptorControllerRailMeasurement) == 8,
              "WaveSculptorControllerRailMeasurement is not 8 bytes");

// Motor Controller Base Addr + 10
typedef struct WaveSculptorFanSpeedMeasurement {
  // Units: %
  // Drive voltage percentage to cooling fan. If this value
  // is above 0%, then the fan should be spinning
  float voltage_percentage;

  // Units: revolutions per minute (rpm)
  // Cooling fan speed in revolutions per minute.
  float fan_rpm;
} WaveSculptorFanSpeedMeasurement;
static_assert(sizeof(WaveSculptorFanSpeedMeasurement) == 8,
              "WaveSculptorFanSpeedMeasurement is not 8 bytes");

// Motor Controller Base Addr + 11
typedef struct WaveSculptorSinkMotorTempMeasurement {
  // Units: C
  // Internal temperature of the motor.
  float motor_temp_c;

  // Units: C
  // Surface temperature of the controller heatsink.
  float heatsink_temp_c;
} WaveSculptorSinkMotorTempMeasurement;
static_assert(sizeof(WaveSculptorSinkMotorTempMeasurement) == 8,
              "WaveSculptorSinkMotorTempMeasurement is not 8 bytes");

// Motor Controller Base Addr + 12
typedef struct WaveSculptorAirInCpuTempMeasurement {
  // Units: C
  // Temperature of the internal processor.
  float processor_temp_c;

  // Units: C
  // Ambient temperature at the ventilation inlet of the controller.
  float air_inlet_temp_c;
} WaveSculptorAirInCpuTempMeasurement;
static_assert(sizeof(WaveSculptorAirInCpuTempMeasurement) == 8,
              "WaveSculptorAirInCpuTempMeasurement is not 8 bytes");

// Motor Controller Base Addr + 13
typedef struct WaveSculptorAirOutCapTempMeasurement {
  // Units: C
  // Ambient temperature of the internal bus capacitors. Unused by 20kW WaveSculptor.
  float unused_capacitor_temp_c;

  // Units: C
  // Ambient air temperature of the ventilation outlet of the controller. Unused in 20kW
  // WaveSculptor.
  float unused_air_out_temp_c;
} WaveSculptorAirOutCapTempMeasurement;
static_assert(sizeof(WaveSculptorAirOutCapTempMeasurement) == 8,
              "WaveSculptorAirOutCapTempMeasurement is not 8 bytes");

// Motor Controller Base Addr + 14
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

typedef union WaveSculptorCanData {
  uint64_t raw;
  WaveSculptorDriveCmd drive_cmd;
  WaveSculptorPowerCmd power_cmd;
  WaveSculptorResetCmd reset_cmd;

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
  WaveSculptorFanSpeedMeasurement fan_speed_measurement;
  WaveSculptorSinkMotorTempMeasurement sink_motor_temp_measurement;
  WaveSculptorAirInCpuTempMeasurement air_in_cpu_temp_measurement;
  WaveSculptorAirOutCapTempMeasurement air_out_cap_temp_measurement;
  WaveSculptorOdometerBusAhMeasurement odometer_bus_ah_measurement;
} WaveSculptorCanData;
