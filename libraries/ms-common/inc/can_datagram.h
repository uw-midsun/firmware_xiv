#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "event_queue.h"
#include "status.h"

#define CAN_DATAGRAM_VERSION 1

#define ID_START_MASK (1 << 7)
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
  NUM_DATAGRAM_DIGEST_EVENTS,
} CanDatagramEvent;

typedef enum {
  CAN_DATAGRAM_MODE_TX = 0,
  CAN_DATAGRAM_MODE_RX,
  NUM_CAN_DATAGRAM_MODES,
} CanDatagramMode;

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

  uint8_t dt_type;
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
  CanDatagramEvent event;
} CanDatagramStorage;

/** Sets the structure field to default values. */
StatusCode can_datagram_init(CanDatagramSettings *settings);

void can_datagram_start(void);

StatusCode can_datagram_rx(uint8_t *data, size_t len, bool start_message);

bool can_datagram_process_event(Event *e);

/** Returns true if the datagram is complete (all data were sent/read). */
bool can_datagram_complete(void);

/** Returns true if the datagram is valid (complete and CRC match). */
bool can_datagram_is_valid(CanDatagram *dt);

/** Encodes the datagram in the buffer. */
int can_datagram_output_bytes(CanDatagram *dt, char *buffer, size_t buffer_len);

/** Computes the CRC32 of the datagram. */
uint32_t can_datagram_compute_crc(void);

CanDatagram *can_datagram_get_datagram(void);

/** Returns true if the ID has the start of datagram field set. */
bool can_datagram_id_start_is_set(unsigned int id);
