#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CAN_DATAGRAM_VERSION 1

#define ID_START_MASK (1 << 7)

typedef enum {
  DATAGRAM_CAN_EVENT_TX = 0,
  DATAGRAM_CAN_EVENT_RX,
  DATAGRAM_CAN_EVENT_FAULT,
  NUM_DATAGRAM_CAN_EVENTS,
} CanDatagramCanEvent;

typedef enum {
  DATAGRAM_EVENT_PROTOCOL_VERSION = NUM_DATAGRAM_CAN_EVENTS + 1;,
  DATAGRAM_EVENT_CRC,
  DATAGRAM_EVENT_DST_LEN,
  DATAGRAM_EVENT_DST,
  DATAGRAM_EVENT_DATA_LEN,
  DATAGRAM_EVENT_DATA,
  DATAGRAM_EVENT_TRAILING,
  DATAGRAM_EVENT_COMPLETE,
  NUM_DATAGRAM_DIGEST_EVENTS,
} CanDatagramEvent;

typedef enum {
  CAN_DATAGRAM_MODE_TX = 0,
  CAN_DATAGRAM_MODE_RX,
  NUM_CAN_DATAGRAM_MODES,
} CanDatagramMode;


StatusCode (*DatagramCallback)(uint8_t *data, size_t len, bool start_message);

typedef struct {
  int protocol_version;
  uint32_t crc;

  uint8_t destination_nodes_len;
  uint8_t *destination_nodes;

  uint32_t data_len;
  uint8_t *data;

  int _crc_bytes_read; //TODO: We may be able to get rid of this section, however error handling may become an issue
  int _crc_bytes_written;
  int _data_length_bytes_read;
  int _data_length_bytes_written;
  uint8_t _destination_nodes_read;
  uint8_t _destination_nodes_written;
  uint16_t _data_bytes_read;
  uint16_t _data_bytes_written;
  uint16_t _data_buffer_size;
  int _reader_state; // If we assume that only one state in action at a time, we can get rid of these
  int _writer_state;
} can_datagram_t;

typedef struct CanDatagramSettings {
  DatagramCallback tx_cb;
  DatagramCallback rx_cb;
  CanDatagramMode mode;
  can_datagram_t dt; // TODO: This may be better to pass as pointer
} CanDatagramSettings;

typedef struct CanDatagramStorage {
  DatagramCallback cb;
  CanDatagramMode mode;
  can_datagram_t * dt;
  CanDatagramEvent state;
}

/** Sets the structure field to default values. */
void can_datagram_init(can_datagram_t *dt);

/** Sets the buffer to use to store destination addresses. */
void can_datagram_set_address_buffer(can_datagram_t *dt, uint8_t *buf);

/** Sets the buffer to use for data storage. */
void can_datagram_set_data_buffer(can_datagram_t *dt, uint8_t *buf, size_t buf_size);

/** Inputs a byte into the datagram. */
void can_datagram_input_byte(can_datagram_t *dt, uint8_t val);

/** Returns true if the datagram is complete (all data were read). */
bool can_datagram_is_complete(can_datagram_t *dt);

/** Returns true if the datagram is valid (complete and CRC match). */
bool can_datagram_is_valid(can_datagram_t *dt);

/** Signals to the parser that we are at the start of a datagram.
 *
 * The start of datagram comes from the Message ID (physical layer).
 */
void can_datagram_start(can_datagram_t *dt);

/** Encodes the datagram in the buffer. */
int can_datagram_output_bytes(can_datagram_t *dt, char *buffer, size_t buffer_len);

/** Computes the CRC32 of the datagram. */
uint32_t can_datagram_compute_crc(can_datagram_t *dt);

/** Returns true if the ID has the start of datagram field set. */
bool can_datagram_id_start_is_set(unsigned int id);
