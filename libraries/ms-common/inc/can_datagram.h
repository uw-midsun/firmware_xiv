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
  CAN_DATAGRAM_MODE_TX = 0,
  CAN_DATAGRAM_MODE_RX,
  NUM_CAN_DATAGRAM_MODES,
} CanDatagramMode;

typedef enum {
  DATAGRAM_STATUS_OK = 0,
  DATAGRAM_STATUS_COMPLETE,
  DATAGRAM_STATUS_ERROR,
} CanDatagramStatus;

typedef struct CanDatagram {
  uint8_t protocol_version;
  uint32_t crc;

  uint8_t dt_type;
  uint8_t destination_nodes_len;
  uint8_t *destination_nodes;
  uint16_t data_len;
  uint8_t *data;
} CanDatagram;

typedef struct CanDatagramSettings {
  CanDatagramCb tx_cb;
  CanDatagramMode mode;

  EventId transition_event;
  EventId repeat_event;
  EventId error_event;

  uint8_t dt_type;
  uint8_t destination_nodes_len;
  uint8_t *destination_nodes;
  uint16_t data_len;
  uint8_t *data;
} CanDatagramSettings;

typedef struct CanDatagramStorage {
  CanDatagram dt;
  CanDatagramMode mode;
  CanDatagramCb tx_cb;

  EventId transition_event;
  EventId repeat_event;
  EventId error_event;

  uint16_t rx_bytes_read;
  uint16_t tx_bytes_sent;
  CanDatagramStatus status;
  bool start;
} CanDatagramStorage;

// Initializes a can datagram instance and prepares for transmitting or receiving
StatusCode can_datagram_init(CanDatagramSettings *settings);

// Called after initialization to start txing datagram messages
StatusCode can_datagram_start_tx(uint8_t *init_data, size_t len);

// Called in the rx handler for datagram messages to process sequential messages
StatusCode can_datagram_rx(uint8_t *data, size_t len, bool start_message);

// Processes datagram state events
bool can_datagram_process_event(Event *e);

// Returns true if the datagram is complete (all data were sent/read)
CanDatagramStatus can_datagram_get_status(void);

// Returns datagram for reading and verification purposes
CanDatagram *can_datagram_get_datagram(void);
