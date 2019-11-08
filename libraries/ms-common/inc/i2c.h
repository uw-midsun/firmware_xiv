#pragma once
// Blocking I2C master driver
// Requires GPIO to be initialized
//
// Supports 7-bit addresses, does not support fast mode plus
#include <stddef.h>
#include <stdint.h>
#include "gpio.h"
#include "i2c_mcu.h"
#include "status.h"

typedef uint8_t I2CAddress;

typedef enum {
  I2C_SPEED_STANDARD = 0,  // 100kHz
  I2C_SPEED_FAST,          // 400 kHz
  NUM_I2C_SPEEDS,
} I2CSpeed;

typedef struct {
  I2CSpeed speed;
  GpioAddress sda;
  GpioAddress scl;
} I2CSettings;

StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings);

// START | ADDR WRITE ACK | DATA ACK | ... | STOP
StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t *rx_data, size_t rx_len);

// START | ADDR READ ACK | DATA ACK | ... | STOP
StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len);

// START | ADDR WRITE ACK | REG ACK | START | ADDR READ ACK | DATA ACK | ... |
// STOP
StatusCode i2c_read_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data, size_t rx_len);

// START | ADDR WRITE ACK | REG ACK | START | ADDR WRITE ACK | DATA ACK | ... |
// STOP
StatusCode i2c_write_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data,
                         size_t tx_len);
