#pragma once

// Definitions of the IDs that can go in the first slot of a babydriver CAN message.

typedef enum {
  // The status message, sent whenever an operation is finished to indicate its status.
  // Message data: uint8 status.
  BABYDRIVER_MESSAGE_STATUS = 0,

  // Raiyan: TODO
  BABYDRIVER_MESSAGE_ADC_READ_COMMAND = 4,

  // Raiyan: TODO
  BABYDRIVER_MESSAGE_ADC_READ_DATA = 5,

  NUM_BABYDRIVER_MESSAGES,
} BabydriverMessageId;
