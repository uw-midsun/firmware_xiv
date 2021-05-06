#include "i2c_read.h"

#include <stdlib.h>
#include <string.h>

#include "babydriver_msg_defs.h"
#include "can_transmit.h"
#include "controller_board_pins.h"
#include "dispatcher.h"
#include "gpio.h"
#include "misc.h"
#include "log.h"

static I2CReadCommand s_storage = { 0 };
static bool s_expecting_message = true;
static uint32_t s_soft_timer_delay;
static size_t bytes_sent = 0;

// The size of the array is rounded up to 266 to account for accessing an array ouy of bounds in the
// loop where the data is transmitted
static uint8_t response[266] = { 0 };
static uint8_t response_index;

// Resets module state to initial state
static void prv_i2c_data_reset(void) {
  s_expecting_message = true;

  bytes_sent = 0;
  s_storage.port = 0;
  s_storage.address = 0;
  s_storage.rx_len = 0;
  s_storage.is_reg = 0;
  s_storage.reg = 0;
}

static void prv_timer_status_callback(SoftTimerId timer_id, void *context) {
  // This function is used to send a STATUS_CODE_OK message
  // The function is in a soft timer call back because there
  // needs to be time between each babydriver message sent

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_STATUS, STATUS_CODE_OK, 0, 0, 0, 0, 0, 0);
}

static void prv_timer_data_callback(SoftTimerId timer_id, void *context) {
  // This function is used to send CAN messages, it is a soft timer
  // callback since there needs to be a delay between messages sent.

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_I2C_READ_DATA, response[bytes_sent],
                          response[bytes_sent + 1], response[bytes_sent + 2],
                          response[bytes_sent + 3], response[bytes_sent + 4],
                          response[bytes_sent + 5], response[bytes_sent + 6]);

  bytes_sent += 7;
//  LOG_DEBUG("%d \n",s_storage.rx_len);
  if (bytes_sent < s_storage.rx_len) {
    soft_timer_start_millis(s_soft_timer_delay, prv_timer_data_callback, NULL, NULL);
  } else {
    soft_timer_start_millis(s_soft_timer_delay, prv_timer_status_callback, NULL, NULL);
    prv_i2c_data_reset();
  }
}

static StatusCode prv_i2c_read_command_callback(uint8_t data[8], void *context, bool *tx_result) {
  *tx_result = false;

  s_storage.port = data[1];
  s_storage.address = data[2];
  s_storage.rx_len = data[3];
  // Stores whether it is a register read, Nonezero for register read, 0 for normal read
  s_storage.is_reg = data[4];

  // Only if it is a register read, store the register
  if (s_storage.is_reg != 0) {
    s_storage.reg = data[5];
  }

  // Initializes I2C module on the I2C port
  if (s_storage.port == I2C_PORT_1) {
    I2CSettings i2c1_settings = {
      .speed = I2C_SPEED_FAST,
      .sda = CONTROLLER_BOARD_ADDR_I2C1_SDA,
      .scl = CONTROLLER_BOARD_ADDR_I2C1_SCL,
    };
    i2c_init(I2C_PORT_1, &i2c1_settings);
  } else if (s_storage.port == I2C_PORT_2) {
    I2CSettings i2c2_settings = {
      .speed = I2C_SPEED_FAST,
      .sda = CONTROLLER_BOARD_ADDR_I2C2_SDA,
      .scl = CONTROLLER_BOARD_ADDR_I2C2_SCL,
    };
    i2c_init(I2C_PORT_2, &i2c2_settings);
  }

  if (s_storage.is_reg != 0) {
    i2c_read_reg(s_storage.port, s_storage.address, s_storage.reg, response, s_storage.rx_len);
  } else {
    i2c_read(s_storage.port, s_storage.address, response, s_storage.rx_len);
  }

  // Transmits the recieved data in blocks of 7
  if (s_storage.rx_len != 0) {
    soft_timer_start_millis(s_soft_timer_delay, prv_timer_data_callback, NULL, NULL);
  } else {
    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_STATUS, STATUS_CODE_OK, 0, 0, 0, 0, 0, 0);
  }

  // Expects a message next
  s_expecting_message = true;

  return STATUS_CODE_OK;
}

StatusCode i2c_read_init(uint32_t timeout_soft_timer) {
  s_soft_timer_delay = timeout_soft_timer;

  return dispatcher_register_callback(BABYDRIVER_MESSAGE_I2C_READ_COMMAND,
                                      prv_i2c_read_command_callback, NULL);
}
