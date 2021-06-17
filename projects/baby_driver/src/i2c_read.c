#include "i2c_read.h"

#include <stdlib.h>
#include <string.h>

#include "babydriver_msg_defs.h"
#include "can_transmit.h"
#include "controller_board_pins.h"
#include "dispatcher.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"

static uint8_t s_rx_len;
static uint32_t s_tx_delay_ms;

// The size of the array is rounded up to 266 to account for accessing an array ouy of bounds in the
// loop where the data is transmitted
static uint8_t s_response[266] = { 0 };

// Resets module state to initial state
static void prv_i2c_data_reset(void) {
  s_rx_len = 0;
  memset(s_response, 0, sizeof(s_response));
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

  uintptr_t bytes_sent = (uintptr_t)context;

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_I2C_READ_DATA, s_response[bytes_sent],
                          s_response[bytes_sent + 1], s_response[bytes_sent + 2],
                          s_response[bytes_sent + 3], s_response[bytes_sent + 4],
                          s_response[bytes_sent + 5], s_response[bytes_sent + 6]);

  bytes_sent += 7;

  if (bytes_sent < s_rx_len) {
    soft_timer_start_millis(s_tx_delay_ms, prv_timer_data_callback, (void *)bytes_sent, NULL);
  } else {
    soft_timer_start_millis(s_tx_delay_ms, prv_timer_status_callback, (void *)bytes_sent, NULL);
    prv_i2c_data_reset();
  }
}

static StatusCode prv_i2c_read_command_callback(uint8_t data[8], void *context, bool *tx_result) {
  *tx_result = false;

  uint8_t port = data[1];
  uint8_t address = data[2];
  s_rx_len = data[3];
  uint8_t is_reg = data[4];
  uint8_t reg = data[5];

  // Initializes I2C module on the I2C port
  if (port == I2C_PORT_1) {
    I2CSettings i2c1_settings = {
      .speed = I2C_SPEED_FAST,
      .sda = CONTROLLER_BOARD_ADDR_I2C1_SDA,
      .scl = CONTROLLER_BOARD_ADDR_I2C1_SCL,
    };
    status_ok_or_return(i2c_init(I2C_PORT_1, &i2c1_settings));
  } else if (port == I2C_PORT_2) {
    I2CSettings i2c2_settings = {
      .speed = I2C_SPEED_FAST,
      .sda = CONTROLLER_BOARD_ADDR_I2C2_SDA,
      .scl = CONTROLLER_BOARD_ADDR_I2C2_SCL,
    };
    status_ok_or_return(i2c_init(I2C_PORT_2, &i2c2_settings));
  }

  if (is_reg != 0) {
    status_ok_or_return(i2c_read_reg(port, address, reg, s_response, s_rx_len));
  } else {
    status_ok_or_return(i2c_read(port, address, s_response, s_rx_len));
  }

  // Transmits the received data in blocks of 7
  if (s_rx_len != 0) {
    soft_timer_start_millis(s_tx_delay_ms, prv_timer_data_callback, (void *)0, NULL);
  } else {
    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_STATUS, STATUS_CODE_OK, 0, 0, 0, 0, 0, 0);
  }

  return STATUS_CODE_OK;
}

StatusCode i2c_read_init(uint32_t tx_delay_ms) {
  s_tx_delay_ms = tx_delay_ms;

  return dispatcher_register_callback(BABYDRIVER_MESSAGE_I2C_READ_COMMAND,
                                      prv_i2c_read_command_callback, NULL);
}
