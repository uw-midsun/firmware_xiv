#pragma once

// Bootloader Datagram IDs for commands and responses

typedef enum {
  // Id used to respond with a status, used by jump to application, update, and flash application
  BOOTLOADER_DATAGRAM_STATUS_RESPONSE = 0,

  // Id used by client to query for specific boards
  BOOTLOADER_DATAGRAM_QUERY_COMMAND = 1,

  // Id used to respond to a query
  BOOTLOADER_DATAGRAM_QUERY_RESPONSE = 2,

  // Id used by client to send a ping command. To check the boards for life.
  BOOTLOADER_DATAGRAM_PING_COMMAND = 3,

  // Id used to respond to a ping command, the data will be the device board id
  BOOTLOADER_DATAGRAM_PING_RESPONSE = 4,

  // Id used by client to jump to application
  BOOTLOADER_DATAGRAM_JUMP_TO_APP = 5,

  // Id used by client to command an update to name
  BOOTLOADER_DATAGRAM_UPDATE_NAME = 6,

  // Id used by client to command an update to id
  BOOTLOADER_DATAGRAM_UPDATE_ID = 7,

  // Id used by client to flash an application
  BOOTLOADER_DATAGRAM_FLASH_APPLICATION_META = 8,

  BOOTLOADER_DATAGRAM_FLASH_APPLICATION_DATA = 9,

  NUM_BOOTLOADER_DATAGRAMS,
} BootloaderDatagramId;
