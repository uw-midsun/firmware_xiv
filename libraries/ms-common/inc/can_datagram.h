#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "event_queue.h"
#include "status.h"

#define CAN_DATAGRAM_VERSION 1
#define DATAGRAM_RX_BUFFER_LEN 256

typedef StatusCode (*CanDatagramCb)(uint8_t *data, size_t len, bool start_message);

typedef enum {
  CAN_DATAGRAM_EVENT_RX = 0,
  CAN_DATAGRAM_EVENT_TX,
  CAN_DATAGRAM_EVENT_FAULT,
  NUM_CAN_DATAGRAM_EVENTS,
} CanDatagramCanEvent;

typedef enum {
  DATAGRAM_EVENT_PROTOCOL_VERSION = NUM_CAN_DATAGRAM_EVENTS + 1,
  DATAGRAM_EVENT_CRC,
  DATAGRAM_EVENT_DST_LEN,
  DATAGRAM_EVENT_DT_TYPE,
  DATAGRAM_EVENT_DST,
  DATAGRAM_EVENT_DATA_LEN,
  DATAGRAM_EVENT_DATA,
  DATAGRAM_EVENT_COMPLETE,
  DATAGRAM_EVENT_ERROR,
  NUM_DATAGRAM_DIGEST_EVENTS,
} CanDatagramEvent;

typedef enum {
  CAN_DATAGRAM_MODE_TX = 0,
  CAN_DATAGRAM_MODE_RX,
  NUM_CAN_DATAGRAM_MODES,
} CanDatagramMode;

typedef enum {
  DATAGRAM_STATUS_OK = 0,
  DATAGRAM_STATUS_COMPLETE,
  DATAGRAM_STATUS_ERROR,
} CanDatagramStatus;

typedef enum {
  // (TODO: SOFT_415) update datagram types
  CAN_DATAGRAM_TYPE_A = 0,
  CAN_DATAGRAM_TYPE_B,
  CAN_DATAGRAM_TYPE_C,
  CAN_DATAGRAM_TYPE_D,
  NUM_CAN_DATAGRAM_TYPES,
} CanDatagramType;

typedef struct RxBufStore {
  uint8_t data[8];
  size_t len;
} RxBufStore;

typedef struct DatagramRxBuffer {
  RxBufStore *head;
  RxBufStore *tail;
  RxBufStore buf[DATAGRAM_RX_BUFFER_LEN];
} DatagramRxBuffer;

typedef struct CanDatagram {
  uint8_t protocol_version;
  uint32_t crc;

  uint8_t dt_type;
  uint8_t destination_nodes_len;
  uint8_t *destination_nodes;
  uint16_t data_len;
  uint8_t *data;  // union needed here with u64?s
} CanDatagram;

typedef struct CanDatagramSettings {
  CanDatagramCb tx_cb;
  CanDatagramMode mode;

  CanDatagramType dt_type;
  uint8_t destination_nodes_len;
  uint8_t *destination_nodes;
  uint16_t data_len;
  uint8_t *data;
} CanDatagramSettings;

typedef struct CanDatagramStorage {
  CanDatagram dt;
  CanDatagramMode mode;
  CanDatagramCb tx_cb;  // Add watchdog error handler?
  uint16_t rx_bytes_read;
  uint16_t tx_bytes_sent;
  CanDatagramStatus status;
  bool start;
} CanDatagramStorage;

/** Sets the structure field to default values. */
StatusCode can_datagram_init(CanDatagramSettings *settings);

StatusCode can_datagram_start_tx(uint8_t *init_data, size_t len);

StatusCode can_datagram_rx(uint8_t *data, size_t len, bool start_message);

bool can_datagram_process_event(Event *e);

/** Returns true if the datagram is complete (all data were sent/read). */
CanDatagramStatus can_datagram_get_status(void);

/** Returns true if the datagram is valid (complete and CRC match). */
bool can_datagram_is_valid(CanDatagram *dt);

/** Computes the CRC32 of the datagram. */
uint32_t can_datagram_compute_crc(void);

CanDatagram *can_datagram_get_datagram(void);
