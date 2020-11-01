#pragma once

#include <stdint.h>

#include "adt7476a_fan_controller.h"
#include "adt7476a_fan_controller_defs.h"
#include "i2c.h"
#include "soft_timer.h"
#include "status.h"

#include "cell_sense.h"

#define BMS_FAN_CTRL_I2C_PORT_1 BMS_PERIPH_I2C_PORT
#define BMS_FAN_CTRL_I2C_PORT_2 TBD
#define NUM_BMS_PORTS 2
#define ADT_7476A_NUM_FANS 4
#define NUM_FANS_PER_OUTPUT 2
#define ADT_7476A_INTERRUPT_MASK_OFFSET 2
#define MAX_BATTERY_TEMP 43
#define MAX_FAN_SPEED 100

typedef struct FanStorage {
  uint16_t speed;
  AfeReadings *readings;
  StatusCode status;
  StatusCode statuses[ADT_7476A_NUM_FANS];
  uint8_t i2c_write_addr;
  uint8_t i2c_read_addr;
} FanStorage;

typedef struct FanControlSettings {
  I2CSettings i2c_settings;
  GpioItCallback callback;
  void *callback_context;
  uint32_t poll_interval_ms;
  uint8_t i2c_write_addr;
  uint8_t i2c_read_addr;
} FanControlSettings;

StatusCode fan_control_init(FanControlSettings *settings, FanStorage *storage);
