#include "can_datagram.h"
#include <stdlib.h>
#include <string.h>
#include "crc32.h"
#include "fsm.h"
#include "log.h"
#include "watchdog.h"

#define PROTOCOL_VERSION_SIZE_BYTES 1
#define CRC_SIZE_BYTES 4
#define DT_TYPE_SIZE_BYTES 1
#define DEST_LEN_SIZE_BYTES 1
#define MAX_DEST_NODES_SIZE_BYTES 255
#define DATA_LEN_SIZE_BYTES 2
#define MAX_DATA_SIZE_BYTES 2048
#define MAX_CAN_TX 4

#define MAX_CRC_SIZE_BYTES 24

#define RX_WATCHDOG_TIMEOUT_MS 25
#define CAN_BUFFER_SIZE 8

static Fsm s_dt_fsm;
static CanDatagramStorage s_store;
static WatchdogStorage s_watchdog;

static DatagramRxBuffer s_rx_msg_buf;
static uint8_t s_can_buffer[CAN_BUFFER_SIZE];
static size_t s_rx_data_len;
static int s_data_write_count;

// If we want to have multiple instances of can datagram stores then this will need to be changed
// Forward declare FSM states and transitions
FSM_DECLARE_STATE(state_idle);
FSM_DECLARE_STATE(state_protocol_version);
FSM_DECLARE_STATE(state_crc);
FSM_DECLARE_STATE(state_dt_type);
FSM_DECLARE_STATE(state_dst_len);
FSM_DECLARE_STATE(state_dst);
FSM_DECLARE_STATE(state_data_len);
FSM_DECLARE_STATE(state_data);

FSM_STATE_TRANSITION(state_idle) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_PROTOCOL_VERSION, state_protocol_version);
}

FSM_STATE_TRANSITION(state_protocol_version) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_CRC, state_crc);
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_PROTOCOL_VERSION, state_protocol_version);
}

FSM_STATE_TRANSITION(state_crc) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_DT_TYPE, state_dt_type);
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_CRC, state_crc);
}

FSM_STATE_TRANSITION(state_dt_type) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_DST_LEN, state_dst_len);
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_DT_TYPE, state_dt_type);
}

FSM_STATE_TRANSITION(state_dst_len) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_DST_LEN, state_dst_len);
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_DST, state_dst);
}

FSM_STATE_TRANSITION(state_dst) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_DST, state_dst);
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_DATA_LEN, state_data_len);
}

FSM_STATE_TRANSITION(state_data_len) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_DATA_LEN, state_data_len);
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_DATA, state_data);
}

FSM_STATE_TRANSITION(state_data) {
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_DATA, state_data);
  FSM_ADD_TRANSITION(DATAGRAM_EVENT_COMPLETE, state_idle);
}

static int count;

// RX Circular buffer functions
static void prv_init_rx_buffer(void) {
  s_rx_msg_buf.head = s_rx_msg_buf.buf;
  s_rx_msg_buf.tail = s_rx_msg_buf.head;
  s_data_write_count = 0;
  count = 0;
}

static void prv_write_rx_buffer(uint8_t *data, size_t len) {
  if (s_rx_msg_buf.head - s_rx_msg_buf.buf > DATAGRAM_RX_BUFFER_LEN) {
    s_rx_msg_buf.head = s_rx_msg_buf.buf;
  }
  (s_rx_msg_buf.head)->len = len;
  memcpy(s_rx_msg_buf.head->data, data, len);
  s_rx_msg_buf.head++;
  s_data_write_count++;
  count++;
}

static bool prv_read_rx_buffer(uint8_t **data, size_t *len) {
  if (s_data_write_count < 1) { // Check that data exists
    return false;
  }
  if (s_rx_msg_buf.tail - s_rx_msg_buf.buf > DATAGRAM_RX_BUFFER_LEN) {
    s_rx_msg_buf.tail = s_rx_msg_buf.buf;
  }
  *len = (s_rx_msg_buf.tail)->len;
  *data = (s_rx_msg_buf.tail)->data;

  s_rx_msg_buf.tail++;
  s_data_write_count--;
  return true;
}

static uint32_t prv_can_datagram_compute_crc(void) {
  uint32_t crc;
  uint8_t stream[MAX_CRC_SIZE_BYTES];
  uint8_t *write = stream;
  CanDatagram *dt = &s_store.dt;

  crc32_init();

  // CRC for data and dst_nodes
  uint32_t dst_crc = crc32_arr(dt->destination_nodes, dt->destination_nodes_len);
  uint32_t data_crc = crc32_arr(dt->data, dt->data_len);
  
  // Copy metadata to stream, append dst and data crc
  memcpy(write, &dt->dt_type, DT_TYPE_SIZE_BYTES);
  write += DT_TYPE_SIZE_BYTES;
  memcpy(write, &dt->destination_nodes_len, DEST_LEN_SIZE_BYTES);
  write += DEST_LEN_SIZE_BYTES;
  memcpy(write, &dst_crc, sizeof(uint32_t));
  write += sizeof(uint32_t);
  memcpy(write, &dt->data_len, DATA_LEN_SIZE_BYTES);
  write += DATA_LEN_SIZE_BYTES;
  memcpy(write, &data_crc, sizeof(uint32_t));
  write += sizeof(uint32_t);

  crc = crc32_arr(stream, (size_t)(write - stream));
  LOG_DEBUG("CRC %u\n", crc);
  return crc;
}

// BEGIN FSM STATE FUNCTIONS
static void prv_process_protocol_version(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("protocol_version\n");
  CanDatagram *dt = context;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memcpy(s_can_buffer, &dt->protocol_version, PROTOCOL_VERSION_SIZE_BYTES);
    s_store.tx_cb(s_can_buffer, PROTOCOL_VERSION_SIZE_BYTES, false);
    event_raise_no_data(DATAGRAM_EVENT_CRC);
  } else {
    uint8_t *rx_data;
    size_t len;
    // Check if data has been written yet, if not, re-raise
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(DATAGRAM_EVENT_PROTOCOL_VERSION);
    } else {
      memcpy(&dt->protocol_version, rx_data, PROTOCOL_VERSION_SIZE_BYTES);
      event_raise_no_data(DATAGRAM_EVENT_CRC);
    }
  }
}

static void prv_process_crc(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("crc\n");
  CanDatagram *dt = context;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memcpy(s_can_buffer, &dt->crc, CRC_SIZE_BYTES);
    s_store.tx_cb(s_can_buffer, CRC_SIZE_BYTES, false);
    event_raise_no_data(DATAGRAM_EVENT_DT_TYPE);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(DATAGRAM_EVENT_CRC);
    } else {
      memcpy(&dt->crc, rx_data, CRC_SIZE_BYTES);
      event_raise_no_data(DATAGRAM_EVENT_DT_TYPE);
    }
  }
}

static void prv_process_dt_type(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("dt_type\n");
  CanDatagram *dt = context;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memcpy(s_can_buffer, &dt->dt_type, DT_TYPE_SIZE_BYTES);
    s_store.tx_cb(s_can_buffer, sizeof(dt->dt_type), false);
    event_raise_no_data(DATAGRAM_EVENT_DST_LEN);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(DATAGRAM_EVENT_DT_TYPE);
    } else {
      memcpy(&dt->dt_type, rx_data, DT_TYPE_SIZE_BYTES);
      event_raise_no_data(DATAGRAM_EVENT_DST_LEN);
    }
  }
}


static void prv_process_dst_len(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("dst_len\n");
  CanDatagram *dt = context;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memcpy(s_can_buffer, &dt->destination_nodes_len, DEST_LEN_SIZE_BYTES);
    s_store.tx_cb(s_can_buffer, sizeof(dt->destination_nodes_len), false);
    event_raise_no_data(DATAGRAM_EVENT_DST);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(DATAGRAM_EVENT_DST_LEN);
    } else {
      memcpy(&dt->destination_nodes_len, rx_data, DEST_LEN_SIZE_BYTES);
      event_raise_no_data(DATAGRAM_EVENT_DST);
    }
  }
}

static void prv_process_dst(Fsm *fsm, const Event *e, void *context) {
  //LOG_DEBUG("dst\n");
  CanDatagram *dt = context;
  uint8_t dst_len = dt->destination_nodes_len;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    uint8_t dst_bytes_to_send = 0;
    // TX max of 4 messages at a time
    for (int msg = 0; msg < MAX_CAN_TX; msg++) {
      dst_bytes_to_send = (dst_len - s_store.tx_bytes_sent < CAN_BUFFER_SIZE) ?
       (dst_len - s_store.tx_bytes_sent) : CAN_BUFFER_SIZE;
      memcpy(s_can_buffer, dt->destination_nodes + s_store.tx_bytes_sent, (size_t)dst_bytes_to_send);
      s_store.tx_bytes_sent += dst_bytes_to_send;
      s_store.tx_cb(s_can_buffer, dst_bytes_to_send, false);
      if(s_store.tx_bytes_sent == dst_len) {
        event_raise_no_data(DATAGRAM_EVENT_DATA_LEN);
        s_store.tx_bytes_sent = 0;
        return;
      }
    }
    event_raise_no_data(DATAGRAM_EVENT_DST);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(DATAGRAM_EVENT_DST);
    } else {
      memcpy(dt->destination_nodes + s_store.rx_bytes_read, rx_data, len);
      s_store.rx_bytes_read += len;
      if (s_store.rx_bytes_read == dt->destination_nodes_len) {
        s_store.rx_bytes_read = 0;
        event_raise_no_data(DATAGRAM_EVENT_DATA_LEN);
      } else {
        event_raise_no_data(DATAGRAM_EVENT_DST);
      }
    }
  }
}

static void prv_process_data_len(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("data len\n");
  CanDatagram *dt = context;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memcpy(s_can_buffer, &dt->data_len, DATA_LEN_SIZE_BYTES);
    s_store.tx_cb(s_can_buffer, sizeof(dt->data_len), false);
    event_raise_no_data(DATAGRAM_EVENT_DATA);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(DATAGRAM_EVENT_DATA_LEN);
    } else {
      memcpy(&dt->data_len, rx_data, DATA_LEN_SIZE_BYTES);
      event_raise_no_data(DATAGRAM_EVENT_DATA);
    }
  }
}

static void prv_process_data(Fsm *fsm, const Event *e, void *context) {
  //LOG_DEBUG("data\n");
  CanDatagram *dt = context;
  uint16_t data_len = dt->data_len;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    uint8_t data_bytes_to_send = 0;
    // TX max of 4 messages at a time
    for (int msg = 0; msg < MAX_CAN_TX; msg++) {
      data_bytes_to_send = (data_len - s_store.tx_bytes_sent < CAN_BUFFER_SIZE)
                               ? (data_len - s_store.tx_bytes_sent)
                               : CAN_BUFFER_SIZE;
      memcpy(s_can_buffer, dt->data + s_store.tx_bytes_sent, (size_t)data_bytes_to_send);
      s_store.tx_bytes_sent += data_bytes_to_send;
      s_store.tx_cb(s_can_buffer, data_bytes_to_send, false);
      if (s_store.tx_bytes_sent == data_len) {
        event_raise_no_data(DATAGRAM_EVENT_COMPLETE);
        s_store.tx_bytes_sent = 0;
        return;
      }
    }
    event_raise_no_data(DATAGRAM_EVENT_DATA);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(DATAGRAM_EVENT_DATA);
    } else {
      memcpy(dt->data + s_store.rx_bytes_read, rx_data, len);
      s_store.rx_bytes_read += len;
      if (s_store.rx_bytes_read >= dt->data_len) {
        s_store.event = DATAGRAM_EVENT_COMPLETE;
        s_store.rx_bytes_read = 0;
        event_raise_no_data(DATAGRAM_EVENT_COMPLETE);
      } else {
        event_raise_no_data(DATAGRAM_EVENT_DATA);
      }
    }
  }
}

static void prv_init_fsm(void *context) {
  fsm_init(&s_dt_fsm, "can_dt_fsm", &state_idle, context);
  fsm_state_init(state_protocol_version, prv_process_protocol_version);
  fsm_state_init(state_crc, prv_process_crc);
  fsm_state_init(state_dt_type, prv_process_dt_type);
  fsm_state_init(state_dst_len, prv_process_dst_len);
  fsm_state_init(state_dst, prv_process_dst);
  fsm_state_init(state_data_len, prv_process_data_len);
  fsm_state_init(state_data, prv_process_data);
}

static void prv_rx_watchdog_expiry_cb(void *context) {
  LOG_DEBUG("Watchdog expired");
}

StatusCode can_datagram_init(CanDatagramSettings *settings) {
  if (settings->mode >= NUM_CAN_DATAGRAM_MODES) {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_store.mode = settings->mode;
  s_store.event = DATAGRAM_EVENT_PROTOCOL_VERSION;

  // Datagram initialization
  CanDatagram *dt = &s_store.dt;
  memset(&s_store.dt, 0, sizeof(s_store.dt));
  // (TODO: SOFT-415) check against valid datagram types once created
  dt->dt_type = settings->dt_type; 
  dt->destination_nodes_len = settings->destination_nodes_len;
  dt->data_len = settings->data_len;
  dt->destination_nodes = settings->destination_nodes;
  dt->data = settings->data;
  dt->crc = prv_can_datagram_compute_crc();
  
  // Mode specific setup
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    if (settings->tx_cb == NULL) {
      return STATUS_CODE_INVALID_ARGS;
    }
    s_store.tx_cb = settings->tx_cb;
    dt->protocol_version =
        CAN_DATAGRAM_VERSION;  // what to do with different protocol version values?
    uint8_t init = 0;
  } else {
    prv_init_rx_buffer();
    memset(dt->destination_nodes, 0, dt->destination_nodes_len);
    memset(dt->data, 0, dt->data_len);
  }

  prv_init_fsm((void *)dt);
  return STATUS_CODE_OK;
}

StatusCode can_datagram_start_tx(uint8_t *init_data, size_t len) {
  // do error checking here before commencing
  if(s_store.mode == CAN_DATAGRAM_MODE_TX && len <= CAN_BUFFER_SIZE) {
    s_store.start = true;
    s_store.tx_cb(init_data, len, true);
    event_raise_no_data(DATAGRAM_EVENT_PROTOCOL_VERSION);
  } else {
    return STATUS_CODE_INVALID_ARGS;
  }
    return STATUS_CODE_OK;
}

CanDatagram *can_datagram_get_datagram(void) {
  return &s_store.dt;
}

StatusCode can_datagram_rx(uint8_t *data, size_t len, bool start_message) {
  if (len > CAN_BUFFER_SIZE) {
    return STATUS_CODE_OUT_OF_RANGE;
  }
  if (s_store.mode != CAN_DATAGRAM_MODE_RX) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (start_message) {
    s_store.start = true;
    event_raise_no_data(DATAGRAM_EVENT_PROTOCOL_VERSION);
    // watchdog_start(&s_watchdog, RX_WATCHDOG_TIMEOUT_MS, prv_rx_watchdog_expiry_cb, NULL);
    return STATUS_CODE_OK;
  }
  // watchdog_kick(&s_watchdog);
  prv_write_rx_buffer(data, len);
  return STATUS_CODE_OK;
  watchdog_start(&s_watchdog, RX_WATCHDOG_TIMEOUT_MS, prv_rx_watchdog_expiry_cb, NULL);
}

bool can_datagram_complete(void) {
  return s_store.event == DATAGRAM_EVENT_COMPLETE;
}

bool can_datagram_process_event(Event *e) {
  return fsm_process_event(&s_dt_fsm, e);
}

bool can_datagram_id_start_is_set(unsigned int id) {
  return id & ID_START_MASK;
}
