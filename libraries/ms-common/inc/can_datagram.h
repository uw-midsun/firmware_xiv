#pragma once
// CAN DATAGRAM PROTOCOL LIBRARY
//
// Allows for transmitting and receiving variable length messages
// over the fixed-latency CAN bus. 
//
// Requires event queue, interrupts, soft timers and crc32 to be init'd
//
// A "datagram" refers to the object in which data and the 
// associated metadata are grouped. See the design document at 
// https://uwmidsun.atlassian.net/l/c/fJ1gAPxP
//
// Usage:
// The Datagram settings struct must be initialized with either
// For Tx:
// 	- All information fields filled
// 	- Data buffers for destination nodes and message data
// For Rx:
// 	- Data and destination node buffers large enough to hold
// 	  however much data is sent

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "event_queue.h"
#include "status.h"


#define CAN_DATAGRAM_VERSION 1

// Callback used to tx a datagram message, used to handle all CAN transmission
// Called in each state as data becomes ready to be transmitted
typedef StatusCode (*CanDatagramTxCb)(uint8_t *data, size_t len, bool start_message);


typedef enum {
  CAN_DATAGRAM_MODE_TX = 0,
  CAN_DATAGRAM_MODE_RX,
  NUM_CAN_DATAGRAM_MODES,
} CanDatagramMode;

typedef enum {
  DATAGRAM_STATUS_ACTIVE = 0,
  DATAGRAM_STATUS_COMPLETE,
  DATAGRAM_STATUS_ERROR,
  NUM_DATAGRAM_STATUSES,
} CanDatagramStatus;

typedef struct CanDatagram {
  uint8_t protocol_version;
  uint32_t crc;

  uint8_t dgram_type;
  uint8_t destination_nodes_len;
  uint8_t *destination_nodes;
  uint16_t data_len;
  uint8_t *data;
} CanDatagram;

typedef struct CanDatagramSettings {
  CanDatagramTxCb tx_cb;
  CanDatagramMode mode;

  EventId tx_event;
  EventId rx_event;
  EventId repeat_event;
  EventId error_event;

  uint8_t dgram_type;
  uint8_t destination_nodes_len;
  uint8_t *destination_nodes;
  uint16_t data_len;
  uint8_t *data;
} CanDatagramSettings;

typedef struct CanDatagramStorage {
  CanDatagram dgram;
  CanDatagramMode mode;
  CanDatagramTxCb tx_cb;

  EventId tx_event;
  EventId rx_event;
  EventId repeat_event;
  EventId error_event;

  size_t rx_bytes_read;
  size_t tx_bytes_sent;
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

