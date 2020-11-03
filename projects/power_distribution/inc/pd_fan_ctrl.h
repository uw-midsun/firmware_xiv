#pragma once

// Control fans for front and rear power distro via ADT7476A

#include "pd_fan_ctrl_defs.h"
#include "pin_defs.h"
#include "soft_timer.h"
#include "status.h"

typedef struct FanCtrlStorage {
  GpioAddress enc_vent_therm_pin;
  GpioAddress dcdc_therm_pin;
  GpioAddress pot_pin;
  I2CPort i2c;
  AdtPwmPort fan_pwm1;
  AdtPwmPort fan_pwm2;
} FanCtrlStorage;

typedef struct FanCtrlSettings {
  I2CPort i2c;
  AdtPwmPort fan_pwm1;
  AdtPwmPort fan_pwm2;  // If rear PD, set this val to dcdc pwm
} FanCtrlSettings;

// Initialize fan control for power distro, indicating front or rear
StatusCode pd_fan_ctrl_init(FanCtrlSettings *settings, bool is_front_pd);
