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

  // The spi_exchange metadata IDs
  // Python will send two metadata messages
  // First message data: uint8 id, port, mode, tx_len, rx_len, cs_port, cs_pin, use_cs
  // Second message data: uint8 id, uint32 baudrate
  BABYDRIVER_MESSAGE_SPI_EXCHANGE_METADATA_1 = 10,
  BABYDRIVER_MESSAGE_SPI_EXCHANGE_METADATA_2 = 11,

  // The spi_exchange ID for receiving and sending spi data
  // Data will be received in ceil(tx_len/7) messages and sent out in ceil(rx_len/7) messages
  // Message data: uint8 ID, 7 * uint8 data
  BABYDRIVER_MESSAGE_SPI_EXCHANGE_TX_DATA = 12,
  BABYDRIVER_MESSAGE_SPI_EXCHANGE_RX_DATA = 13,

  NUM_BABYDRIVER_MESSAGES,
} BabydriverMessageId;
