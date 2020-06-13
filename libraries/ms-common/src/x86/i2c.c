#include "i2c.h"
#include "log.h"
StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  } else if (settings->speed >= NUM_I2C_SPEEDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C speed.");
  }
  LOG_DEBUG("Note this is an x86 version of I2C");
  return STATUS_CODE_OK;
}

StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t *rx_data, size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  LOG_DEBUG("Reading Data...\n");
  for (size_t i = 0; i < rx_len; i++) {
    // Insert dummy data
    if (i % 2 == 0) {
      rx_data[i] = 1;
    }
    LOG_DEBUG("0x%x\n", rx_data[i]);
  }
  return STATUS_CODE_OK;
}

StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  LOG_DEBUG("Sending Data...\n");
  for (size_t i = 0; i < tx_len; i++) {
    LOG_DEBUG("0x%x\n", tx_data[i]);
  }
  return STATUS_CODE_OK;
}

StatusCode i2c_read_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data,
                        size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  LOG_DEBUG("Reading Register Address...\n");
  for (size_t i = 0; i < rx_len; i++) {
    // Insert dummy data
    if (i % 2 == 0) {
      rx_data[i] = 1;
    }
    LOG_DEBUG("0x%x\n", rx_data[i]);
  }
  return STATUS_CODE_OK;
}

StatusCode i2c_write_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data,
                         size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  LOG_DEBUG("Writing Register Address...\n");
  for (size_t i = 0; i < tx_len; i++) {
    LOG_DEBUG("0x%x\n", tx_data[i]);
  }
  return STATUS_CODE_OK;
}
