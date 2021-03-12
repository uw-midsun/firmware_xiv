#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "event_queue.h"
#include "status.h"

#define CAN_DATAGRAM_VERSION 1

#define ID_START_MASK (1 << 7)

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


typedef struct CanDatagram{
  uint8_t protocol_version;
  uint32_t crc;

  uint8_t destination_nodes_len;
  uint8_t *destination_nodes;

  uint16_t data_len;
  uint8_t *data;
} CanDatagram;

typedef struct CanDatagramSettings {
  CanDatagramCb tx_cb;
  CanDatagramMode mode;
} CanDatagramSettings;

typedef struct CanDatagramStorage {
  CanDatagram dt;
  CanDatagramMode mode;
  CanDatagramCb tx_cb; // Add watchdog error handler?
  uint8_t rx_bytes_to_read;
  uint16_t rx_bytes_read;
  CanDatagramEvent event;
} CanDatagramStorage;

/** Sets the structure field to default values. */
StatusCode can_datagram_init(CanDatagramSettings *settings);

void can_datagram_start_tx(void);

StatusCode can_datagram_rx(uint8_t *data, size_t len, bool start_message);

bool can_datagram_process_event(Event *e);

/** Sets the buffer to use to store destination addresses. */
StatusCode can_datagram_set_address_buffer(uint8_t *dst, size_t num_dst_nodes);

/** Sets the buffer to use for data storage. */
StatusCode can_datagram_set_data_buffer(uint8_t *data, size_t data_len);

/** Returns true if the datagram is complete (all data were read). */
bool can_datagram_tx_complete(void);

/** Returns true if the datagram is valid (complete and CRC match). */
bool can_datagram_is_valid(CanDatagram *dt);

/** Signals to the parser that we are at the start of a datagram.
 *
 * The start of datagram comes from the Message ID (physical layer).
 */
void can_datagram_start(CanDatagram *dt);

/** Encodes the datagram in the buffer. */
int can_datagram_output_bytes(CanDatagram *dt, char *buffer, size_t buffer_len);

/** Computes the CRC32 of the datagram. */
uint32_t can_datagram_compute_crc(CanDatagram *dt);

/** Returns true if the ID has the start of datagram field set. */
bool can_datagram_id_start_is_set(unsigned int id);
