#pragma once

// Definitions of the IDs that can go in the first slot of a babydriver CAN message.

typedef enum {
  // The status message, sent whenever an operation is finished to indicate its status.
  // Message data: uint8 status.
  BABYDRIVER_MESSAGE_STATUS = 0,

  // The gpio set message, received when Python gpio_set function is called to indicate that a gpio
  // pin needs to be set to a specific state.
  // Message data: uint8 id, uint8 port, uint8 pin, uint8 state
  BABYDRIVER_MESSAGE_GPIO_SET = 1,

  // The i2c write command message, received when Python i2c_write function is called to indicate
  // that data must be written over i2c and provides information about the number of messages that
  // must be received.
  // Message data: uint8 id, uint8 port, uint8 address, uint8 tx_len, uint8 is_reg, uint8 reg
  BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND = 8,

  // The i2c write data message, received when Python i2c_write function is called to indicate the
  // data that must be written over i2c and provides 7 bytes of information per message.
  // Message data: uint8 id, 7 * uint8 data
  BABYDRIVER_MESSAGE_I2C_WRITE_DATA = 9,

  NUM_BABYDRIVER_MESSAGES,
} BabydriverMessageId;
