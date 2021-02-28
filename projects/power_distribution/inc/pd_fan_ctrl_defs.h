#pragma once
#include "adt7476a_fan_controller.h"
#include "adt7476a_fan_controller_defs.h"

// I2C info - based on atd7276a pin13 high on powerup
#define ADT7476A_I2C_ADDRESS 0x2E

// TODO(SOFT-373): - confirm pwm ports with hardware
// Pwm info - needs confirmation
#define FRONT_PD_PWM_1 ADT_PWM_PORT_1
#define FRONT_PD_PWM_2 ADT_PWM_PORT_2
#define REAR_ENC_VENT_PWM ADT_PWM_PORT_1
#define REAR_DCDC_PWM ADT_PWM_PORT_2

// Fan control info
#define FRONT_FAN_CONTROL_REFRESH_PERIOD_MILLIS 100
#define REAR_FAN_CONTROL_REFRESH_PERIOD_MILLIS 100

