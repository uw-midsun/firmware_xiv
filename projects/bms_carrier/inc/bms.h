#pragma once

#include <stdint.h>

#include "debouncer.h"
#include "i2c.h"
#include "status.h"

#include "bps_heartbeat.h"
#include "cell_sense.h"
#include "current_sense.h"
#include "fan_control.h"
#include "relay_sequence.h"

#define BMS_PERIPH_I2C_PORT I2C_PORT_2
#define BMS_PERIPH_I2C_SDA_PIN \
  { GPIO_PORT_B, 11 }
#define BMS_PERIPH_I2C_SCL_PIN \
  { GPIO_PORT_B, 10 }
#define BMS_FAN_ALERT_PIN \
  { GPIO_PORT_A, 9 }

#define BMS_IO_EXPANDER_I2C_ADDR 0x40

#define BMS_FAN_CTRL_1_I2C_ADDR 0x5E
#define BMS_FAN_CTRL_2_I2C_ADDR 0x5F
#define NUM_BMS_FAN_CTRLS 2

typedef struct BmsStorage {
  RelayStorage relay_storage;
  CurrentStorage current_storage;
  AfeReadings afe_readings;
  LtcAfeStorage ltc_afe_storage;
  CellSenseStorage cell_storage;
  FanStorage fan_storage_1;
  FanStorage fan_storage_2;
  DebouncerStorage killswitch_storage;
  BpsStorage bps_storage;
} BmsStorage;
