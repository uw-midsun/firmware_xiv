#include "can_datagram.h"
#include <stdlib.h>
#include <string.h>
#include "crc32.h"
#include "fsm.h"
#include "log.h"
#include "watchdog.h"

#define MAX_DATA_LEN 2048
#define MAX_DEST_NODES 255  // This could be changed?
#define RX_WATCHDOG_TIMEOUT_MS 25
#define CAN_BUFFER_SIZE 8
#define CRC_SIZE_BYTES 4
#define DATA_LENGTH_SIZE_BYTES 2
#define DEST_NODE_MAX_SIZE 255
#define DATA_MAX_SIZE 2048

#define CRC_TEST_VALUE 0x121212

static Fsm s_dt_fsm;
static CanDatagramStorage s_store;
static WatchdogStorage s_watchdog;

static DatagramRxBuffer s_rx_msg_buf;
static uint8_t s_can_buffer[CAN_BUFFER_SIZE];
static size_t s_rx_data_len;

static int s_data_write_count;
// If we want to have multiple instances of can datagram stores then this will need to be changed

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

// RX Circular buffer functions
static void prv_init_rx_buffer(void) {
  s_rx_msg_buf.head = s_rx_msg_buf.buf;
  s_rx_msg_buf.tail = s_rx_msg_buf.head;
}

static void prv_write_rx_buffer(uint8_t *data, size_t len) {
  if (s_rx_msg_buf.head - s_rx_msg_buf.buf > DATAGRAM_RX_BUFFER_LEN) {
    s_rx_msg_buf.head = s_rx_msg_buf.buf;
  }
  (s_rx_msg_buf.head)->len = len;
  for (uint8_t i = 0; i < len; i++) {
    (s_rx_msg_buf.head)->data[i] = data[i];
  }
  s_rx_msg_buf.head++;
  s_data_write_count++;
}

static bool prv_read_rx_buffer(uint8_t **data, size_t *len) {
  if (s_data_write_count < 1) {
    return false;
  }
  if (s_rx_msg_buf.tail - s_rx_msg_buf.buf > DATAGRAM_RX_BUFFER_LEN) {
    s_rx_msg_buf.tail = s_rx_msg_buf.buf;
  }
  *len = (s_rx_msg_buf.tail)->len;
  *data = (s_rx_msg_buf.tail)->data;
  // for(int i = 0; i < 8; i++) {
  //     LOG_DEBUG("RX DATA: %d\n", (*data)[i]);
  // }
  s_rx_msg_buf.tail++;
  s_data_write_count++;
  return true;
}

// BEGIN FSM STATE FUNCTIONS
static void prv_process_protocol_version(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("protocol_version\n");
  s_store.event = DATAGRAM_EVENT_CRC;  // TODO: Do we need to know current state for anything?
  CanDatagram *dt = context;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memset(
        s_can_buffer, 0,
        CAN_BUFFER_SIZE * sizeof(uint8_t));  // Reset can datagram buffer - not strictly necessary
    s_can_buffer[0] = dt->protocol_version;
    s_store.tx_cb(s_can_buffer, sizeof(dt->protocol_version), false);
    event_raise_no_data(DATAGRAM_EVENT_CRC);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data,
                            &len)) {  // Check if data has been written yet, if not, re-raise
      event_raise_no_data(DATAGRAM_EVENT_PROTOCOL_VERSION);
    } else {
      dt->protocol_version = rx_data[0];
            event_raise_no_data(DATAGRAM_EVENT_CRC);
    }
  }
}

static void prv_process_crc(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("crc\n");
  s_store.event = DATAGRAM_EVENT_DT_TYPE;
  CanDatagram *dt = context;
  dt->crc = CRC_TEST_VALUE;
  memset(s_can_buffer, 0, CAN_BUFFER_SIZE * sizeof(uint8_t));
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    for (uint8_t byte = 0; byte < CRC_SIZE_BYTES; byte++) {
      s_can_buffer[byte] = dt->crc >> (24 - 8 * byte) & 0xFF;
    }
    s_store.tx_cb(s_can_buffer, CRC_SIZE_BYTES, false);
    event_raise_no_data(DATAGRAM_EVENT_DT_TYPE);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(DATAGRAM_EVENT_CRC);
    } else {
      for (uint8_t byte = 0; byte < len; byte++) {
        dt->crc = dt->crc << 8 | rx_data[byte];
      }
      event_raise_no_data(DATAGRAM_EVENT_DT_TYPE);
    }
  }
}

static void prv_process_dt_type(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("dt_type\n");
  s_store.event = DATAGRAM_EVENT_DST_LEN;
  CanDatagram *dt = context;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memset(s_can_buffer, 0, CAN_BUFFER_SIZE * sizeof(uint8_t));
    s_can_buffer[0] = dt->dt_type;
    s_store.tx_cb(s_can_buffer, sizeof(dt->dt_type), false);
    event_raise_no_data(DATAGRAM_EVENT_DST_LEN);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(DATAGRAM_EVENT_DT_TYPE);
    } else {
      dt->dt_type = rx_data[0];
      event_raise_no_data(DATAGRAM_EVENT_DST_LEN);
    }
  }
}

static void prv_process_dst_len(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("dst_len\n");
  CanDatagram *dt = context;
  s_store.event = DATAGRAM_EVENT_DST;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memset(s_can_buffer, 0, CAN_BUFFER_SIZE * sizeof(uint8_t));
    s_can_buffer[0] = dt->destination_nodes_len;
    s_store.tx_cb(s_can_buffer, sizeof(dt->destination_nodes_len), false);
    event_raise_no_data(DATAGRAM_EVENT_DST);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(DATAGRAM_EVENT_DST_LEN);
    } else {
      dt->destination_nodes_len = rx_data[0];
            event_raise_no_data(DATAGRAM_EVENT_DST);
    }
  }
}

static void prv_process_dst(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("dst\n");
  s_store.event = DATAGRAM_EVENT_DATA_LEN;
  CanDatagram *dt = context;
  uint8_t dst_len = dt->destination_nodes_len;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    uint8_t dst_bytes_sent = 0;
    uint8_t dst_bytes_to_send = 0;
    memset(s_can_buffer, 0, CAN_BUFFER_SIZE * sizeof(uint8_t));
    while (dst_bytes_sent < dst_len) {
      dst_bytes_to_send = (dst_len - dst_bytes_sent < CAN_BUFFER_SIZE) ? (dst_len - dst_bytes_sent)
                                                                       : CAN_BUFFER_SIZE;
      for (uint8_t byte = 0; byte < dst_bytes_to_send; byte++) {
        s_can_buffer[byte] = dt->destination_nodes[dst_bytes_sent];
        dst_bytes_sent++;
      }
      s_store.tx_cb(s_can_buffer, dst_bytes_to_send, false);
    }
    event_raise_no_data(DATAGRAM_EVENT_DATA_LEN);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(DATAGRAM_EVENT_DST);
    } else {
      for (uint8_t byte = 0; byte < len; byte++) {
        dt->destination_nodes[s_store.rx_bytes_read] = rx_data[byte];
        s_store.rx_bytes_read++;
      }
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
  LOG_DEBUG("data_len\n");
  CanDatagram *dt = context;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memset(s_can_buffer, 0, CAN_BUFFER_SIZE * sizeof(uint8_t));
    for (uint8_t byte = 0; byte < DATA_LENGTH_SIZE_BYTES; byte++) {
      s_can_buffer[byte] = dt->data_len >> (24 - 8 * byte) & 0xFF;
    }
    s_store.tx_cb(s_can_buffer, sizeof(dt->data_len), false);
    event_raise_no_data(DATAGRAM_EVENT_DATA);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(DATAGRAM_EVENT_DATA_LEN);
    } else {
      for (uint8_t byte = 0; byte < DATA_LENGTH_SIZE_BYTES; byte++) {
        dt->data_len = dt->data_len | rx_data[byte] << (8 * byte);
      }
      event_raise_no_data(DATAGRAM_EVENT_DATA);
    }
  }
}

static void prv_process_data(Fsm *fsm, const Event *e, void *context) {
  //LOG_DEBUG("data\n");
  CanDatagram *dt = context;
  uint8_t data_len = dt->data_len;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    uint8_t data_bytes_sent = 0;
    uint8_t data_bytes_to_send;
    memset(s_can_buffer, 0, CAN_BUFFER_SIZE * sizeof(uint8_t));
    while (data_bytes_sent < data_len) {
      data_bytes_to_send = (data_len - data_bytes_sent < CAN_BUFFER_SIZE)
                               ? data_len - data_bytes_sent
                               : CAN_BUFFER_SIZE;
      for (int byte = 0; byte < data_bytes_to_send; byte++) {
        s_can_buffer[byte] = dt->destination_nodes[data_bytes_sent];
        data_bytes_sent++;
      }
      s_store.tx_cb(s_can_buffer, data_bytes_to_send, false);
    }
    s_store.event = DATAGRAM_EVENT_COMPLETE;
    event_raise_no_data(DATAGRAM_EVENT_COMPLETE);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(DATAGRAM_EVENT_DATA);
    } else {
      for (uint8_t byte = 0; byte < len; byte++) {
        dt->data[s_store.rx_bytes_read] = rx_data[byte];
        s_store.rx_bytes_read++;
      }
      if (s_store.rx_bytes_read == dt->data_len) {
        event_raise_no_data(DATAGRAM_EVENT_COMPLETE);
        s_store.event = DATAGRAM_EVENT_COMPLETE;
        s_store.rx_bytes_read = 0;
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
  // Zero datagram
  CanDatagram *dt = &s_store.dt;
  memset(&s_store.dt, 0, sizeof(s_store.dt));
  if (settings->mode >= NUM_CAN_DATAGRAM_MODES) {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_store.mode = settings->mode;

  // Datagram initialization
  dt->dt_type =
      settings->dt_type;  // (TODO: SOFT-415) check against valid datagram types once created
  dt->destination_nodes_len = settings->destination_nodes_len;  // Error checking for buffers?
  dt->data_len = settings->data_len;
  dt->destination_nodes = settings->destination_nodes;
  dt->data = settings->data;
  LOG_DEBUG("LEN A: %d, LEN: %d\n", dt->data_len, settings->data_len);

  // Mode specific setup
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    if (settings->tx_cb == NULL) {
      return STATUS_CODE_INVALID_ARGS;
    }
    s_store.tx_cb = settings->tx_cb;
    dt->protocol_version =
        CAN_DATAGRAM_VERSION;  // what to do with different protocol version values?
    uint8_t init = 0;
    s_store.tx_cb(&init, 0, true);  // TODO: move to proto, use proper start message
  } else {
    prv_init_rx_buffer();
    memset(dt->destination_nodes, 0, dt->destination_nodes_len);
    memset(dt->data, 0, dt->data_len);
  }

  prv_init_fsm((void *)dt);
  crc32_init();
  return STATUS_CODE_OK;
}

void can_datagram_start(void) {
  // do error checking here before commencing
  event_raise_no_data(DATAGRAM_EVENT_PROTOCOL_VERSION);
}

StatusCode can_datagram_rx(uint8_t *data, size_t len, bool start_message) {
  if (len > CAN_BUFFER_SIZE) {
    return STATUS_CODE_OUT_OF_RANGE;
  }
  // if (start_message) {
  //   // watchdog_start(&s_watchdog, RX_WATCHDOG_TIMEOUT_MS, prv_rx_watchdog_expiry_cb, NULL);
  //   event_raise_no_data(DATAGRAM_EVENT_PROTOCOL_VERSION);
  //   return STATUS_CODE_OK;
  // } /*else {
  //   // watchdog_kick(&s_watchdog);
  // }*/
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

#if 0
StatusCode can_datagram_compute_crc(void) {
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

#endif
