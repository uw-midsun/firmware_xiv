#pragma once
#include "adt7476a_fan_controller.h"

// I2C info - TODO: Figure out what's needed
#define ADT7476A_I2C_READ_ADDRESS 0x01   // These values need to be set
#define ADT7476A_I2C_WRITE_ADDRESS 0x02  // These values need to be set

// Fan control info
#define FRONT_FAN_CONTROL_REFRESH_PERIOD_MILLIS 100
#define REAR_FAN_CONTROL_REFRESH_PERIOD_MILLIS 100

// Pwm info
#define FRONT_PD_PWM_1 ADT_PWM_PORT_1
#define FRONT_PD_PWM_2 ADT_PWM_PORT_2
#define REAR_ENC_VENT_PWM ADT_PWM_PORT_1
#define REAR_DCDC_PWM ADT_PWM_PORT_2

// Interrupt status register bit definitions
// Used in transmitting fan info over CAN (based on page 22 of ADT7476A data sheet)

// ISR 1 overtemp/overvoltage bits - all shifted left to MSB of u16 fan data
#define FAN_OVERTEMP_TRIGGERED 0x20  // Indicates if overtemp condition triggered
#define DCDC_OVERTEMP 0x10           // DCDC over-temp condition
#define ENC_VENT_OVERTEMP 0x08       // ENCLOSURE VENTILATION over-temp condition
#define VCC_EXCEEDED 0x04            // Indicates that Input voltage high or low limit exceeded
#define VCCP_EXCEEDED 0x02           // Indicates that Input voltage high or low limit exceeded

// ISR 2 fan status bits - will be set if fans drop below threshold speed
// LSB of u16 fan data
#define FAN1_STATUS 0x04
#define FAN2_STATUS 0x08
#define FAN3_STATUS 0x10
#define FAN4_STATUS 0x20
