#include "i2c_write.h"

#include <math.h>
#include <stdlib.h>

#include "can_transmit.h"
#include "dispatcher.h"
#include "gpio.h"
#include "i2c.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define I2C1_SDA \
  { .port = GPIO_PORT_B, .pin = 9 }
#define I2C1_SCL \
  { .port = GPIO_PORT_B, .pin = 8 }
#define I2C2_SDA \
  { .port = GPIO_PORT_B, .pin = 11 }
#define I2C2_SCL \
  { .port = GPIO_PORT_B, .pin = 10 }

#define TIMEOUT_PERIOD_MS 750

static uint8_t message_count = 0;
static uint8_t i2c_command[5];
static uint8_t i2c_metadata[255];

// Resets module state from previous set of messages
// message_count is not included to ensure it is set to 0 if i2c_write or i2c_write_reg fails
void i2c_data_reset() {
  for (uint8_t i = 0; i < 5; i++) {
    i2c_command[i] = 0;
  }
  for (uint8_t j = 0; j <= 254; j++) {
    i2c_metadata[j] = 0;
  }
}

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  message_count = 0;
  i2c_data_reset();
  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_STATUS, STATUS_CODE_TIMEOUT, 0, 0, 0, 0, 0, 0);
}

static StatusCode prv_i2c_write_data_callback(uint8_t data[8], void *context, bool *tx_result) {
  *tx_result = false;

  // Enters only for the command message
  if (message_count == 0) {
    // Start timer to check for timeout error
    soft_timer_start_millis(TIMEOUT_PERIOD_MS, prv_timer_callback, NULL, NULL);
    i2c_data_reset();

    // Checks babydriver_id, port, and tx_len
    if (data[0] != BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND || data[1] >= NUM_I2C_PORTS ||
        data[3] < 7) {
      *tx_result = true;
      return STATUS_CODE_INVALID_ARGS;
    }

    // Stores data from command message
    for (uint8_t i = 0; i < 5; i++) {
      i2c_command[i] = data[i + 1];
    }

    message_count++;

    return STATUS_CODE_OK;

    // Enters only for the final message
  } else if (message_count == ceil(i2c_command[2] / 7)) {
    *tx_result = true;

    // Checks babydriver_id
    if (data[0] != BABYDRIVER_MESSAGE_I2C_WRITE_DATA) {
      message_count = 0;
      return STATUS_CODE_INVALID_ARGS;
    }

    // Stores new data in i2c_metadata
    uint8_t metadata_index = (message_count - 1) * 7;
    for (uint8_t i = 0; i < 7; ++i) {
      i2c_metadata[metadata_index + i] = data[i + 1];
    }

    message_count = 0;

    I2CPort port = i2c_command[0];
    I2CAddress addr = i2c_command[1];
    size_t tx_len = i2c_command[2];
    uint8_t is_reg = i2c_command[3];
    uint8_t reg = i2c_command[4];

    // Initializes i2c with correct port and settings
    if (port == I2C_PORT_1) {
      I2CSettings i2c1_settings = {
        .speed = I2C_SPEED_FAST,
        .sda = I2C1_SDA,
        .scl = I2C1_SCL,
      };
      i2c_init(I2C_PORT_1, &i2c1_settings);
    } else {
      I2CSettings i2c2_settings = {
        .speed = I2C_SPEED_FAST,
        .sda = I2C2_SDA,
        .scl = I2C2_SCL,
      };
      i2c_init(I2C_PORT_2, &i2c2_settings);
    }

    // Sends data over i2c with consideration to whether a register is specified
    if (is_reg) {
      status_ok_or_return(i2c_write_reg(port, addr, reg, i2c_metadata, tx_len));
    } else {
      status_ok_or_return(i2c_write(port, addr, i2c_metadata, tx_len));
    }

    return STATUS_CODE_OK;

  } else {
    // Checks babydriver_id
    if (data[0] != BABYDRIVER_MESSAGE_I2C_WRITE_DATA) {
      message_count = 0;
      *tx_result = true;
      return STATUS_CODE_INVALID_ARGS;
    }

    // Stores data in i2c_metadata
    uint8_t metadata_index = (message_count - 1) * 7;
    for (uint8_t i = 0; i < 7; i++) {
      i2c_metadata[metadata_index + i] = data[i + 1];
    }

    message_count++;

    return STATUS_CODE_OK;
  }
}

StatusCode i2c_write_init(void) {
  status_ok_or_return(dispatcher_register_callback(BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND,
                                                   prv_i2c_write_data_callback, NULL));
  return dispatcher_register_callback(BABYDRIVER_MESSAGE_I2C_WRITE_DATA,
                                      prv_i2c_write_data_callback, NULL);
}
