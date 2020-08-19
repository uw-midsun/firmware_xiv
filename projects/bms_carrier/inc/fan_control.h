#pragma once

#include <stdint.h>

#include "adt7476a_fan_controller.h"
#include "adt7476a_fan_controller_defs.h"
#include "i2c.h"
#include "soft_timer.h"
#include "status.h"

#include "cell_sense.h"

#define BMS_FAN_CTRL_I2C_PORT_1 TBD
#define BMS_FAN_CTRL_I2C_PORT_2 TBD
#define NUM_BMS_FANS 4

typedef struct FanStorage {
  uint16_t speeds[NUM_BMS_FANS];
  StatusCode status;
} FanStorage;

StatusCode fan_control_init(I2CSettings *settings, FanStorage *storage);
