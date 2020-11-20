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

  NUM_BABYDRIVER_MESSAGES,
} BabydriverMessageId;
