#include "can_datagram.h"
// #include <crc/crc32.h>
#include <stdlib.h>
#include <string.h>
#include "fsm.h"
#include "log.h"

#define CAN_TX_BUFFER_SIZE 8
#define MAX_DATA_LEN      2048
#define MAX_DEST_NODES 255 // This could be changed?

static Fsm s_dt_fsm;
static CanDatagramStorage s_store;
static uint8_t s_can_tx_buffer[CAN_TX_BUFFER_SIZE];


// If we want to have multiple instances of can datagram stores then this will need to be changed

FSM_DECLARE_STATE(state_idle);
FSM_DECLARE_STATE(state_protocol_version);
FSM_DECLARE_STATE(state_crc);
FSM_DECLARE_STATE(state_dst_len);
FSM_DECLARE_STATE(state_dst);
FSM_DECLARE_STATE(state_data_len);
FSM_DECLARE_STATE(state_data);


FSM_STATE_TRANSITION(state_idle) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_PROTOCOL_VERSION, state_protocol_version);
}

FSM_STATE_TRANSITION(state_protocol_version) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_CRC, state_crc);
}

FSM_STATE_TRANSITION(state_crc) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_DST_LEN, state_dst_len);
}

FSM_STATE_TRANSITION(state_dst_len) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_DST, state_dst);
}

FSM_STATE_TRANSITION(state_dst) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_DATA_LEN, state_data_len);
}

FSM_STATE_TRANSITION(state_data_len) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_DATA, state_data);
}

FSM_STATE_TRANSITION(state_data) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_COMPLETE, state_idle);
}


// Process for protocol version depending on if store/send requested
static void prv_process_protocol_version(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("protocol_version\n");
  CanDatagram *dt = context;
  memset(s_can_tx_buffer, 0, CAN_TX_BUFFER_SIZE*sizeof(uint8_t)); // Reset can datagram buffer - not strictly necessary
  s_can_tx_buffer[0] = dt->protocol_version;
  s_store.tx_cb(s_can_tx_buffer, sizeof(dt->protocol_version), false);
  event_raise_no_data(DATAGRAM_EVENT_CRC);
}

static void prv_process_crc(Fsm *fsm, const Event *e, void *context) { // TODO: decide if sizeof is ok or should we do #defines
  LOG_DEBUG("crc\n");
  CanDatagram *dt = context;
  memset(s_can_tx_buffer, 0, CAN_TX_BUFFER_SIZE*sizeof(uint8_t));
  for(uint8_t byte = 0; byte < sizeof(dt->crc); byte++) {
    s_can_tx_buffer[byte] = dt->crc >> (24 - 8 * byte);
  }
  s_store.tx_cb(s_can_tx_buffer, sizeof(dt->crc), false);
  event_raise_no_data(DATAGRAM_EVENT_DST_LEN);
}

static void prv_process_dst_len(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("dst_len\n");
  CanDatagram *dt = context;
  memset(s_can_tx_buffer, 0, CAN_TX_BUFFER_SIZE*sizeof(uint8_t));
  s_can_tx_buffer[0] = dt->destination_nodes_len;
  s_store.tx_cb(s_can_tx_buffer, sizeof(dt->destination_nodes_len), false);
  event_raise_no_data(DATAGRAM_EVENT_DST);
}

static void prv_process_dst(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("dst\n");
  CanDatagram *dt = context;
  uint8_t dst_len = dt->destination_nodes_len;
  uint8_t dst_bytes_sent = 0;
  uint8_t dst_bytes_to_send = 0; 
  memset(s_can_tx_buffer, 0, CAN_TX_BUFFER_SIZE*sizeof(uint8_t));
  while(dst_bytes_sent < dst_len) {
    dst_bytes_to_send = (dst_len - dst_bytes_sent < CAN_TX_BUFFER_SIZE) ? (dst_len - dst_bytes_sent) : CAN_TX_BUFFER_SIZE;
    LOG_DEBUG("DST BYTES TO SEND: %d\n", dst_bytes_to_send);
    for(uint8_t byte = 0; byte < dst_bytes_to_send; byte++) {
	    s_can_tx_buffer[byte] = dt->destination_nodes[dst_bytes_sent];
	    dst_bytes_sent++;
    }
    s_store.tx_cb(s_can_tx_buffer, sizeof(dt->destination_nodes_len), false);
  }
  event_raise_no_data(DATAGRAM_EVENT_DATA_LEN);
}

static void prv_process_data_len(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("data_len\n");
  CanDatagram *dt = context;
  memset(s_can_tx_buffer, 0, CAN_TX_BUFFER_SIZE*sizeof(uint8_t));
  s_can_tx_buffer[0] = dt->data_len;
  s_store.tx_cb(s_can_tx_buffer, sizeof(dt->data_len), false);
  event_raise_no_data(DATAGRAM_EVENT_DATA);
}	

static void prv_process_data(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("data\n");
  CanDatagram *dt = context;
  uint8_t data_len = dt->data_len;
  uint8_t data_bytes_sent = 0;
  uint8_t data_bytes_to_send;
  memset(s_can_tx_buffer, 0, CAN_TX_BUFFER_SIZE*sizeof(uint8_t));
  while(data_bytes_sent < data_len) {
    data_bytes_to_send = (data_len - data_bytes_sent < CAN_TX_BUFFER_SIZE) ?
	    data_len - data_bytes_sent : CAN_TX_BUFFER_SIZE;
    for(int byte = 0; byte < data_bytes_to_send; byte++) {
	    s_can_tx_buffer[byte] = dt->destination_nodes[data_bytes_sent];
	    data_bytes_sent++;
    }
    s_store.tx_cb(s_can_tx_buffer, sizeof(dt->destination_nodes_len), false);
  }
  s_store.state = DATAGRAM_EVENT_COMPLETE;
  event_raise_no_data(DATAGRAM_EVENT_COMPLETE);
}

static void prv_init_fsm(void * context) {
	fsm_init(&s_dt_fsm, "tx_fsm", &state_idle, context);
	fsm_state_init(state_protocol_version, prv_process_protocol_version);
	fsm_state_init(state_crc, prv_process_crc);
	fsm_state_init(state_dst_len, prv_process_dst_len);
	fsm_state_init(state_dst, prv_process_dst);
	fsm_state_init(state_data_len, prv_process_data_len);
	fsm_state_init(state_data, prv_process_data);
}

StatusCode can_datagram_init(CanDatagramSettings * settings) {
  // Populate storage
  // s_store.rx_cb = settings->rx_cb;
  CanDatagram * dt = &s_store.dt;
  memset(&s_store.dt, 0, sizeof(s_store.dt));

  s_store.tx_cb = settings->tx_cb;
  s_store.mode = settings->mode;
  s_store.state = DATAGRAM_EVENT_PROTOCOL_VERSION;

  dt->protocol_version = CAN_DATAGRAM_VERSION;

  if(s_store.mode == CAN_DATAGRAM_MODE_TX) { // Is this the best way to send init or should we send from protoc v
	  s_store.tx_cb(NULL, 0, true);
  }

  prv_init_fsm((void*)dt);
  return STATUS_CODE_OK;
}

StatusCode can_datagram_set_data_buffer(uint8_t *data, size_t data_len) {
  if (data_len > MAX_DATA_LEN) {
    return STATUS_CODE_OUT_OF_RANGE;
  } else {
    s_store.dt.data_len = (uint16_t)data_len;
    s_store.dt.data = data;
  }
  return STATUS_CODE_OK;
}

StatusCode can_datagram_set_address_buffer(uint8_t *dst, size_t num_dst_nodes) { // do we want these in the settings?
  if (num_dst_nodes > MAX_DEST_NODES) {
    return STATUS_CODE_OUT_OF_RANGE;
  } else {
    s_store.dt.destination_nodes_len = (uint16_t)num_dst_nodes;
    s_store.dt.destination_nodes = dst;
  }
  return STATUS_CODE_OK;
} 

void can_datagram_start_tx(void) {
  // do error checking here before commencing
  event_raise_no_data(DATAGRAM_EVENT_PROTOCOL_VERSION);
}

bool can_datagram_tx_complete(void) {
  return s_store.state == DATAGRAM_EVENT_COMPLETE;
}

bool can_datagram_process_event(Event *e) {
  return fsm_process_event(&s_dt_fsm, e);
} 

#if 0
void can_datagram_set_address_buffer(can_datagram_t *dt, uint8_t *buf) {
  dt->destination_nodes = buf;
}

void can_datagram_set_data_buffer(can_datagram_t *dt, uint8_t *buf, size_t buf_size) {
  dt->data = buf;
  dt->_data_buffer_size = buf_size;
}

void can_datagram_input_byte(can_datagram_t *dt, uint8_t val) {
  switch (dt->_reader_state) {
    case STATE_PROTOCOL_VERSION:
      dt->protocol_version = val;
      dt->_reader_state = STATE_CRC;
      break;

    case STATE_CRC:
      dt->crc = (dt->crc << 8) | val;
      dt->_crc_bytes_read++;

      if (dt->_crc_bytes_read == 4) {
        dt->_reader_state = STATE_DST_LEN;
      }
      break;

    case STATE_DST_LEN: /* Destination nodes list length */
      dt->destination_nodes_len = val;
      dt->_reader_state = STATE_DST;
      break;

    case STATE_DST: /* Destination nodes */
      dt->destination_nodes[dt->_destination_nodes_read] = val;
      dt->_destination_nodes_read++;

      if (dt->_destination_nodes_read == dt->destination_nodes_len) {
        dt->_reader_state = STATE_DATA_LEN;
      }
      break;

    case STATE_DATA_LEN: /* Data length, MSB */
      dt->data_len = (dt->data_len << 8) | val;
      dt->_data_length_bytes_read++;

      if (dt->_data_length_bytes_read == 4) {
        dt->_reader_state = STATE_DATA;
      }

      break;

    case STATE_DATA: /* Data */
      dt->data[dt->_data_bytes_read] = val;
      dt->_data_bytes_read++;

      if (dt->_data_bytes_read == dt->data_len) {
        dt->_reader_state = STATE_TRAILING;
      }

      if (dt->_data_buffer_size == dt->_data_bytes_read) {
        dt->_reader_state = STATE_TRAILING;
      }

      break;

    default:

      /* Don't change state, stay here forever. */
      break;
  }
}

bool can_datagram_is_complete(can_datagram_t *dt) {
  return dt->_reader_state > 0 && dt->_data_bytes_read == dt->data_len &&
         dt->_data_length_bytes_read == 4;
}

bool can_datagram_is_valid(can_datagram_t *dt) {
  return can_datagram_compute_crc(dt) == dt->crc && dt->protocol_version == CAN_DATAGRAM_VERSION;
}

void can_datagram_start(can_datagram_t *dt) {
  dt->_reader_state = STATE_PROTOCOL_VERSION;
  dt->_crc_bytes_read = 0;
  dt->_destination_nodes_read = 0;
  dt->_data_bytes_read = 0;
  dt->_data_length_bytes_read = 0;
}

int can_datagram_output_bytes(can_datagram_t *dt, char *buffer, size_t buffer_len) {
  size_t i;
  for (i = 0; i < buffer_len; i++) {
    switch (dt->_writer_state) {
      case STATE_PROTOCOL_VERSION:
        buffer[i] = dt->protocol_version;
        dt->_writer_state = STATE_CRC;
        break;

      case STATE_CRC:
        buffer[i] = dt->crc >> (24 - 8 * dt->_crc_bytes_written);
        dt->_crc_bytes_written++;

        if (dt->_crc_bytes_written == 4) {
          dt->_writer_state = STATE_DST_LEN;
        }
        break;

      case STATE_DST_LEN: /* Destination node length */
        buffer[i] = dt->destination_nodes_len;
        dt->_writer_state = STATE_DST;
        break;

      case STATE_DST: /* Destination nodes */
        buffer[i] = dt->destination_nodes[dt->_destination_nodes_written];
        dt->_destination_nodes_written++;

        if (dt->_destination_nodes_written == dt->destination_nodes_len) {
          dt->_writer_state = STATE_DATA_LEN;
        }
        break;

      case STATE_DATA_LEN: /* Data length MSB first */
        buffer[i] = dt->data_len >> (24 - 8 * dt->_data_length_bytes_written);
        dt->_data_length_bytes_written++;

        if (dt->_data_length_bytes_written == 4) {
          dt->_writer_state = STATE_DATA;
        }
        break;

      case STATE_DATA: /* Data */
        /* If already finished, just return. */
        if (dt->_data_bytes_written == dt->data_len) {
          return 0;
        }

        buffer[i] = dt->data[dt->_data_bytes_written];
        dt->_data_bytes_written++;

        /* If we don't have anymore data to send, return written byte
         * count. */
        if (dt->_data_bytes_written == dt->data_len) {
          return i + 1;
        }
        break;
    }
  }

  return buffer_len;
}

uint32_t can_datagram_compute_crc(can_datagram_t *dt) {
  uint32_t crc;
  uint8_t tmp[4];
  crc = crc32(0, &dt->destination_nodes_len, 1);
  crc = crc32(crc, &dt->destination_nodes[0], dt->destination_nodes_len);

  /* data_len is not in network endianess, correct that before CRC update. */
  tmp[0] = (dt->data_len >> 24) & 0xff;
  tmp[1] = (dt->data_len >> 16) & 0xff;
  tmp[2] = (dt->data_len >> 8) & 0xff;
  tmp[3] = (dt->data_len >> 0) & 0xff;

  crc = crc32(crc, tmp, 4);
  crc = crc32(crc, &dt->data[0], dt->data_len);
  return crc;
}

#endif
bool can_datagram_id_start_is_set(unsigned int id) {
  return id & ID_START_MASK;
}
