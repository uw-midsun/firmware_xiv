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

  NUM_BABYDRIVER_MESSAGES,
} BabydriverMessageId;
