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

  // The gpio_get command message, sent whenever gpio_get() is called in Python.
  // Message data: uint8 port, uint8 pin.
  BABYDRIVER_MESSAGE_GPIO_GET_COMMAND = 2,

  // The gpio_get data message, sent from firmware when it gets the state of the pin requested
  // Message data: uint8 state
  BABYDRIVER_MESSAGE_GPIO_GET_DATA = 3,

  // The adc_read command message, sent whenever adc_read() is called in python
  // Message data: uint8 port, uint8 pin, uint8 is_raw
  BABYDRIVER_MESSAGE_ADC_READ_COMMAND = 4,

  // The adc_read data message, sent from firmware when it gets data from the port/pin requested
  // Message data: uint8 low_byte, uint8 high_byte
  BABYDRIVER_MESSAGE_ADC_READ_DATA = 5,

  // The i2c write command message, received when Python i2c_write function is called to indicate
  // that data must be written over i2c and provides information about the number of messages that
  // must be received.
  // Message data: uint8 id, uint8 port, uint8 address, uint8 tx_len, uint8 is_reg, uint8 reg
  BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND = 8,

  // The i2c write data message, received when Python i2c_write function is called to indicate the
  // data that must be written over i2c and provides 7 bytes of information per message.
  // Message data: uint8 id, 7 * uint8 data
  BABYDRIVER_MESSAGE_I2C_WRITE_DATA = 9,

  // The gpio interrupts register command message, received when the Python gpio interrupts function
  // is called to register a gpio interrupt.
  // Message data: uint8 id, uint8 port, uint8 pin, uint8 edge
  BABYDRIVER_MESSAGE_GPIO_IT_REGISTER_COMMAND = 14,

  // The gpio interrupts unregister command message, received when the Python gpio interrupts
  // function is called to unregister a previously registered gpio interrupt.
  // Message data: uint8 id, uint8 port, uint8 pin
  BABYDRIVER_MESSAGE_GPIO_IT_UNREGISTER_COMMAND = 15,

  // The gpio interrupts message, sent when a gpio interrupt is triggered.
  // Message data: uint8 id, uint8 port, uint8 pin, uint8 edge
  BABYDRIVER_MESSAGE_GPIO_IT_INTERRUPT = 16,

  NUM_BABYDRIVER_MESSAGES,
} BabydriverMessageId;
