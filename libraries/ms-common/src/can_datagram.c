#include "can_datagram.h"
// #include <crc/crc32.h>
#include <stdlib.h>
#include <string.h>
#include "fsm.h"
#include "log.h"
#include "watchdog.h"

#define MAX_DATA_LEN   2048
#define MAX_DEST_NODES 255 // This could be changed?
#define RX_WATCHDOG_TIMEOUT_MS 25
#define CAN_BUFFER_SIZE 8
#define CRC_SIZE_BYTES 4
#define U16_DATA_LENGTH_SIZE_BYTES 2

static Fsm s_dt_fsm;
static CanDatagramStorage s_store;
static uint8_t s_can_buffer[CAN_BUFFER_SIZE];
static WatchdogStorage s_watchdog;


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
  if(s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memset(s_can_buffer, 0, CAN_BUFFER_SIZE*sizeof(uint8_t)); // Reset can datagram buffer - not strictly necessary
    s_can_buffer[0] = dt->protocol_version;
    s_store.tx_cb(s_can_buffer, sizeof(dt->protocol_version), false);
    event_raise_no_data(DATAGRAM_EVENT_CRC);
  } else {
    dt->protocol_version = s_can_buffer[0];
    s_store.event = DATAGRAM_EVENT_CRC;
  }
}

static void prv_process_crc(Fsm *fsm, const Event *e, void *context) { // TODO: decide if sizeof is ok or should we do #defines
  LOG_DEBUG("crc\n");
  CanDatagram *dt = context;
  memset(s_can_buffer, 0, CAN_BUFFER_SIZE*sizeof(uint8_t));
  if(s_store.mode == CAN_DATAGRAM_MODE_TX) {
    for(uint8_t byte = 0; byte < CRC_SIZE_BYTES; byte++) {
      s_can_buffer[byte] = dt->crc >> (24 - 8 * byte);
    }
    s_store.tx_cb(s_can_buffer, CRC_SIZE_BYTES, false);
    event_raise_no_data(DATAGRAM_EVENT_DST_LEN);
  } else {
    for(uint8_t byte = 0; byte < CRC_SIZE_BYTES; byte++) {
      dt->crc = dt->crc << 8 | s_can_buffer[byte];
    }
    s_store.event = DATAGRAM_EVENT_DST_LEN;
  }
}

static void prv_process_dst_len(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("dst_len\n");
  CanDatagram *dt = context;
  if(s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memset(s_can_buffer, 0, CAN_BUFFER_SIZE*sizeof(uint8_t));
    s_can_buffer[0] = dt->destination_nodes_len;
    s_store.tx_cb(s_can_buffer, sizeof(dt->destination_nodes_len), false);
    event_raise_no_data(DATAGRAM_EVENT_DST);
  } else {
    dt->destination_nodes_len = s_can_buffer[0];
    s_store.event = DATAGRAM_EVENT_DST;
  }
}

static void prv_process_dst(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("dst\n");
  CanDatagram *dt = context;
  uint8_t dst_len = dt->destination_nodes_len;
  if(s_store.mode == CAN_DATAGRAM_MODE_TX) {
    uint8_t dst_bytes_sent = 0;
    uint8_t dst_bytes_to_send = 0; 
    memset(s_can_buffer, 0, CAN_BUFFER_SIZE*sizeof(uint8_t));
    while(dst_bytes_sent < dst_len) {
      dst_bytes_to_send = (dst_len - dst_bytes_sent < CAN_BUFFER_SIZE) ? (dst_len - dst_bytes_sent) : CAN_BUFFER_SIZE;
      for(uint8_t byte = 0; byte < dst_bytes_to_send; byte++) {
        s_can_buffer[byte] = dt->destination_nodes[dst_bytes_sent];
        dst_bytes_sent++;
      }
      s_store.tx_cb(s_can_buffer, sizeof(dt->destination_nodes_len), false);
    }
    event_raise_no_data(DATAGRAM_EVENT_DATA_LEN);
  } else {
    for(uint8_t byte = 0; byte < s_store.rx_bytes_to_read; byte++) {
      dt->destination_nodes[s_store.rx_bytes_read] = s_can_buffer[byte];
      s_store.rx_bytes_read++;
    }
    if (s_store.rx_bytes_read == dt->destination_nodes_len) {
      s_store.event = DATAGRAM_EVENT_DATA_LEN;
      s_store.rx_bytes_read = 0;
    }
  } 
}

static void prv_process_data_len(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("data_len\n");
  CanDatagram *dt = context;
  if(s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memset(s_can_buffer, 0, CAN_BUFFER_SIZE*sizeof(uint8_t));
    s_can_buffer[0] = dt->data_len;
    s_store.tx_cb(s_can_buffer, sizeof(dt->data_len), false);
    event_raise_no_data(DATAGRAM_EVENT_DATA);
  } else {
    for(uint8_t byte = 0; byte < U16_DATA_LENGTH_SIZE_BYTES; byte++) {
      dt->data_len = dt->data_len << 8 | s_can_buffer[byte];
    }
    s_store.event = DATAGRAM_EVENT_DATA;
  }
}	

static void prv_process_data(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("data\n");
  CanDatagram *dt = context;
  uint8_t data_len = dt->data_len;
  if(s_store.mode == CAN_DATAGRAM_MODE_TX) {
    uint8_t data_bytes_sent = 0;
    uint8_t data_bytes_to_send;
    memset(s_can_buffer, 0, CAN_BUFFER_SIZE*sizeof(uint8_t));
    while(data_bytes_sent < data_len) {
      data_bytes_to_send = (data_len - data_bytes_sent < CAN_BUFFER_SIZE) ?
        data_len - data_bytes_sent : CAN_BUFFER_SIZE;
      for(int byte = 0; byte < data_bytes_to_send; byte++) {
        s_can_buffer[byte] = dt->destination_nodes[data_bytes_sent];
        data_bytes_sent++;
      }
      s_store.tx_cb(s_can_buffer, sizeof(dt->destination_nodes_len), false);
    }
    s_store.event = DATAGRAM_EVENT_COMPLETE;
    event_raise_no_data(DATAGRAM_EVENT_COMPLETE);
  } else {
    for(uint8_t byte = 0; byte < s_store.rx_bytes_to_read; byte++) {
      dt->destination_nodes[s_store.rx_bytes_read] = s_can_buffer[byte];
      s_store.rx_bytes_read++;
    }
    if (s_store.rx_bytes_read == dt->data_len) {
      s_store.event = DATAGRAM_EVENT_COMPLETE;
      s_store.rx_bytes_read = 0;
    }
  }
}

static void prv_init_fsm(void * context) {
	fsm_init(&s_dt_fsm, "can_dt_fsm", &state_idle, context);
	fsm_state_init(state_protocol_version, prv_process_protocol_version);
	fsm_state_init(state_crc, prv_process_crc);
	fsm_state_init(state_dst_len, prv_process_dst_len);
	fsm_state_init(state_dst, prv_process_dst);
	fsm_state_init(state_data_len, prv_process_data_len);
	fsm_state_init(state_data, prv_process_data);
}

static void prv_rx_watchdog_expiry_cb(void* context) {
  LOG_DEBUG("Watchdog expired");
}

StatusCode can_datagram_init(CanDatagramSettings * settings) {
  // Populate storage
  CanDatagram * dt = &s_store.dt;
  memset(&s_store.dt, 0, sizeof(s_store.dt));
 
  s_store.tx_cb = settings->tx_cb;
  s_store.mode = settings->mode;

  if(s_store.mode == CAN_DATAGRAM_MODE_TX) {
    dt->protocol_version = CAN_DATAGRAM_VERSION; // what to do with different protocol version values?
	  s_store.tx_cb(NULL, 0, true); // TODO: move to proto, use proper start message
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
  if (num_dst_nodes > MAX_DEST_NODES) { // should add bool to see if data/dst has been set
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

StatusCode can_datagram_rx(uint8_t *data, size_t len, bool start_message) {
  if (len > CAN_BUFFER_SIZE) {
    return STATUS_CODE_OUT_OF_RANGE;
  }

  if (start_message) {
    watchdog_start(&s_watchdog, RX_WATCHDOG_TIMEOUT_MS, prv_rx_watchdog_expiry_cb, NULL);
    s_store.event = DATAGRAM_EVENT_PROTOCOL_VERSION;
  } else {
    watchdog_kick(&s_watchdog);
  }

  s_store.rx_bytes_to_read = len;
  for(uint8_t byte = 0; byte < len; byte++) {
    s_can_buffer[byte] = data[byte];
  }
  event_raise_no_data(s_store.event);
  return STATUS_CODE_OK;
}

bool can_datagram_tx_complete(void) {
  return s_store.event == DATAGRAM_EVENT_COMPLETE;
}

bool can_datagram_process_event(Event *e) {
  return fsm_process_event(&s_dt_fsm, e);
} 



#if 0

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
