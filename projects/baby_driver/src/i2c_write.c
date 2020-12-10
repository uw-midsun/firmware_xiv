#include "i2c_write.h"

#include <stdlib.h>
#include <string.h>

#include "can_transmit.h"
#include "controller_board_pins.h"
#include "dispatcher.h"
#include "gpio.h"
#include "misc.h"
#include "watchdog.h"

#define MAX_DATA_BYTES_TRANSMITTED 255

static I2CWriteCommand s_storage = { 0 };

static WatchdogStorage s_watchdog;

static uint8_t s_metadata_index = 0;
static uint8_t s_i2c_metadata[MAX_DATA_BYTES_TRANSMITTED];
static bool s_expecting_command_message = true;

// Resets module state to initial state
static void prv_i2c_data_reset(void) {
  s_metadata_index = 0;
  memset(s_i2c_metadata, 0, sizeof(s_i2c_metadata));
  s_expecting_command_message = true;

  s_storage.port = 0;
  s_storage.address = 0;
  s_storage.tx_len = 0;
  s_storage.is_reg = 0;
  s_storage.reg = 0;
}

// Watchdog callback to tx timeout error
static void prv_expiry_callback(void *context) {
  // Only tx's timeout error if timeout occurs between data and command message or consecutive data
  // messages
  if (!s_expecting_command_message) {
    prv_i2c_data_reset();
    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_STATUS, STATUS_CODE_TIMEOUT, 0, 0, 0, 0, 0, 0);
  }
}

// Writes data received over i2c
static StatusCode prv_i2c_data_transmit(void) {
  if (s_storage.port == I2C_PORT_1) {
    I2CSettings i2c1_settings = {
      .speed = I2C_SPEED_FAST,
      .sda = CONTROLLER_BOARD_ADDR_I2C1_SDA,
      .scl = CONTROLLER_BOARD_ADDR_I2C1_SCL,
    };
    i2c_init(I2C_PORT_1, &i2c1_settings);
  } else {
    I2CSettings i2c2_settings = {
      .speed = I2C_SPEED_FAST,
      .sda = CONTROLLER_BOARD_ADDR_I2C2_SDA,
      .scl = CONTROLLER_BOARD_ADDR_I2C2_SCL,
    };
    i2c_init(I2C_PORT_2, &i2c2_settings);
  }

  // Sends data over i2c with consideration to whether a register is specified
  if (s_storage.is_reg) {
    status_ok_or_return(i2c_write_reg(s_storage.port, s_storage.address, s_storage.reg,
                                      s_i2c_metadata, s_storage.tx_len));
  } else {
    status_ok_or_return(
        i2c_write(s_storage.port, s_storage.address, s_i2c_metadata, s_storage.tx_len));
  }

  return STATUS_CODE_OK;
}

static StatusCode prv_i2c_write_data_callback(uint8_t data[8], void *context, bool *tx_result) {
  watchdog_kick(&s_watchdog);
  *tx_result = false;

  // Checks whether data message was expected
  if (s_expecting_command_message) {
    *tx_result = true;
    prv_i2c_data_reset();
    return STATUS_CODE_INVALID_ARGS;
  }

  // Stores data from CAN message
  // Upper bound is needed to prevent index from going out of array bounds when tx_len is not
  // divisible by 7
  uint8_t upper_bound = MIN(s_storage.tx_len - s_metadata_index, 7);
  for (uint8_t i = 0; i < upper_bound; i++) {
    s_i2c_metadata[s_metadata_index++] = data[i + 1];
  }

  // Writes data over i2c and resets module state after final data message is received
  if (s_metadata_index == s_storage.tx_len) {
    *tx_result = true;
    prv_i2c_data_transmit();
    prv_i2c_data_reset();
  }

  return STATUS_CODE_OK;
}

static StatusCode prv_i2c_write_command_callback(uint8_t data[8], void *context, bool *tx_result) {
  watchdog_kick(&s_watchdog);
  *tx_result = false;

  s_storage.port = data[1];
  s_storage.address = data[2];
  s_storage.tx_len = data[3];
  s_storage.is_reg = data[4];
  s_storage.reg = data[5];

  // Checks whether port is invalid or whether a command message is expected
  if (s_storage.port >= NUM_I2C_PORTS || !s_expecting_command_message) {
    *tx_result = true;
    prv_i2c_data_reset();
    return STATUS_CODE_INVALID_ARGS;
  }

  // Expects a data message next
  s_expecting_command_message = false;

  return STATUS_CODE_OK;
}

StatusCode i2c_write_init(uint32_t timeout_ms) {
  // For timeout test the the timeout period is adjusted otherwise 750ms is the default timeout
  watchdog_start(&s_watchdog, timeout_ms, prv_expiry_callback, NULL);
  status_ok_or_return(dispatcher_register_callback(BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND,
                                                   prv_i2c_write_command_callback, NULL));
  return dispatcher_register_callback(BABYDRIVER_MESSAGE_I2C_WRITE_DATA,
                                      prv_i2c_write_data_callback, NULL);
}
