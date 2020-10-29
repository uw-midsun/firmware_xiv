#include "i2c.h"
#include "log.h"
#include "stdio.h"

StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  } else if (settings->speed >= NUM_I2C_SPEEDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C speed.");
  }
  LOG_DEBUG("Note this is an x86 version of I2C\n");
  return STATUS_CODE_OK;
}

StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t *rx_data, size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  LOG_DEBUG("Reading %ld bytes over I2C\n", rx_len);
  for (size_t i = 0; i < rx_len; i++) {
    // Insert dummy data
    rx_data[i] = i % 2;
  }
  return STATUS_CODE_OK;
}

StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  LOG_DEBUG("Sending %ld bytes over I2C: \n", tx_len);
  if (LOG_LEVEL_DEBUG >= LOG_LEVEL_VERBOSITY) {
    for (size_t i = 0; i < tx_len; i++) {
      // printf("0x%x ", tx_data[i]);
    }
    // printf("\n");
  }
  return STATUS_CODE_OK;
}

StatusCode i2c_read_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data,
                        size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  LOG_DEBUG("Reading %ld bytes from register %d over I2C\n", rx_len, reg);
  for (size_t i = 0; i < rx_len; i++) {
    // Insert dummy data
    rx_data[i] = i % 2;
  }
  return STATUS_CODE_OK;
}

StatusCode i2c_write_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data,
                         size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  LOG_DEBUG("Writing %ld bytes to register %d over I2C: \n", tx_len, reg);
  if (LOG_LEVEL_DEBUG >= LOG_LEVEL_VERBOSITY) {
    for (size_t i = 0; i < tx_len; i++) {
      // printf("0x%x ", tx_data[i]);
    }
    // printf("\n");
  }
  return STATUS_CODE_OK;
}
