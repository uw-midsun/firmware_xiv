#include "can_datagram.h"
#include <crc/crc32.h>
#include <stdlib.h>
#include <string.h>
#include "fsm.h"

#define NUM_CRC_BYTES 4



static CanDatagramStorage * s_store; // If need to store more than one  can turn this into a table
static can_datagram_t * dt;
// If we want to have multiple instances of can datagram stores then this will need to be changed

FSM_DECLARE_STATE(state_init);
FSM_DECLARE_STATE(state_protocol_version);
FSM_DECLARE_STATE(state_crc);
FSM_DECLARE_STATE(state_dst_len);
FSM_DECLARE_STATE(state_dst);
FSM_DECLARE_STATE(state_data_len);
FSM_DECLARE_STATE(state_data);
FSM_DECLARE_STATE(state_trailing);

FSM_ADD_TRANSITION(DATAGRAM_EVENT_PROTOCOL_VERSION, state_protocol_version);
FSM_ADD_TRANSITION(DATAGRAM_EVENT_CRC, state_crc);
FSM_ADD_TRANSITION(DATAGRAM_EVENT_DST_LEN, state_dst_len);
FSM_ADD_TRANSITION(DATAGRAM_EVENT_DST, state_dst);
FSM_ADD_TRANSITION(DATAGRAM_EVENT_DATA_LEN, state_data_len);
FSM_ADD_TRANSITION(DATAGRAM_EVENT_DATA, state_data);
FSM_ADD_TRANSITION(DATAGRAM_EVENT_TRAILING, state_trailing);

STATE_TRANSITION(state_init) {
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
  FSM_ADD_TRANSITION(NUM_DATAGRAM_DIGEST_EVENTS, state_protocol_version);
}

FSM_STATE_TRANSITION(state_dst) {
  FSM_ADD_TRANSITION(NUM_DATAGRAM_DIGEST_EVENTS, state_protocol_version);
}

FSM_STATE_TRANSITION(state_dst) {
  FSM_ADD_TRANSITION(NUM_DATAGRAM_DIGEST_EVENTS, state_protocol_version);
}

FSM_STATE_TRANSITION(state_dst) {
  FSM_ADD_TRANSITION(NUM_DATAGRAM_DIGEST_EVENTS, state_protocol_version);
}

// Process for protocol version depending on if store/send requested
static void process_protocol_version(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("protocol_version\n");
  uint8_t buffer; // Is there a more efficient way to store this memory ie. static buffer
  if(s_store.mode == CAN_DATAGRAM_MODE_TX){
    buffer = dt->protocol_version;
    s_store.tx_cb(&buffer, 1, false);
  }
  data_state = DATAGRAM_EVENT_CRC;
}

static void process_crc(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("crc\n");
  if(s_store.mode == CAN_DATAGRAM_MODE_TX){
    uint8_t buf[4] = { 0 };
    for(int byte = 0; i < NUM_CRC_BYTES; byte++) {
      buf[i] = dt->crc >> (24 - 8 * byte);
      dt->_crc_bytes_written++;
    }
  }
  data_state = DATAGRAM_EVENT_DST_LEN;
}

static void process_dst_len(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("dst_len\n");
  if(s_store.mode == CAN_DATAGRAM_MODE_TX){

  }
  data_state = DATAGRAM_EVENT_DST;
}

static void process_dst(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("dst\n");
  if(s_store.mode == CAN_DATAGRAM_MODE_TX){

  }
  data_state = NUM_DATAGRAM_DIGEST_EVENTS;
}

static void process_data_len(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("data_len\n");
  if(s_store.mode == CAN_DATAGRAM_MODE_TX){

  }
  data_state = DATAGRAM_EVENT_DATA;
}	

static void process_data(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("data\n");
  if(s_store.mode == CAN_DATAGRAM_MODE_TX){

  }
  data_state = DATAGRAM_EVENT_TRAILING;
}

static void process_trailing(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("trailing\n");
  if(s_store.mode == CAN_DATAGRAM_MODE_TX){

  }
  data_state = DATAGRAM_EVENT_COMPLETE;
}












static void prv_init_fsm(void * context) {
	fsm_init(&fsm, "fsm", &state_init, context);
	fsm_state_init(state_protocol_version, process_protocol_version);
	fsm_state_init(state_crc, process_crc);
	fsm_state_init(state_dst_len, process_dst_len);
	fsm_state_init(state_dst, process_dst);
	fsm_state_init(state_data_len, process_data_len);
	fsm_state_init(state_data, process_data);
	fsm_state_init(state_trailing, process_trailing);
}

can_datagram_t *get_can_datagram(void) {
	return &dt;
}





StatusCode can_datagram_init(CanDatagramSettings * settings) {
  // Populate storage
  s_store.rx_cb = settings->rx_cb;
  s_store.tx_cb = settings->tx_cb;
  s_store.mode = settings->mode;
  dt = &settings->dt;
  s_store.state = DATAGRAM_EVENT_PROTOCOL_VERSION;
  
  if(s_store.mode == CAN_DATAGRAM_MODE_TX) {
	  s_store.tx_cb(NULL, 0, true); // TODO: Update init message
  }

  memset(&settings->dt, 0, sizeof(settingsdt);
  dt->protocol_version = CAN_DATAGRAM_VERSION;
}










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

bool can_datagram_id_start_is_set(unsigned int id) {
  return id & ID_START_MASK;
}
