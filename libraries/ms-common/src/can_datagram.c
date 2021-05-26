#include "can_datagram.h"

#include <stdlib.h>
#include <string.h>

#include "crc32.h"
#include "fsm.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "watchdog.h"

#define PROTOCOL_VERSION_SIZE_BYTES 1
#define CRC_SIZE_BYTES 4
#define DGRAM_TYPE_SIZE_BYTES 1
#define DEST_LEN_SIZE_BYTES 1
#define MAX_DEST_NODES_SIZE_BYTES 255
#define DATA_LEN_SIZE_BYTES 2
#define MAX_DATA_SIZE_BYTES 2048
#define MAX_CAN_TX 4

#define MAX_CRC_STREAM_SIZE_BYTES 24

#define RX_WATCHDOG_TIMEOUT_MS 25
#define CAN_BUFFER_SIZE 8

typedef struct RxBufStore {
  uint8_t data[8];
  size_t len;
} RxBufStore;

typedef struct DatagramRxBuffer {
  RxBufStore *head;
  RxBufStore *tail;
  RxBufStore buf[DATAGRAM_RX_BUFFER_LEN];
} DatagramRxBuffer;

static Fsm s_dgram_fsm;
static CanDatagramStorage s_store;
static WatchdogStorage s_watchdog;

static DatagramRxBuffer s_rx_msg_buf;
static uint8_t s_can_buffer[CAN_BUFFER_SIZE];
static int s_data_write_count;

// Forward declare FSM states and transitions
// Each state can transition to next on completion of processing
// or return to itself if data has not been received or has yet
// to be processed

// Idle state
FSM_DECLARE_STATE(state_idle);
// Protocol Version processed
FSM_DECLARE_STATE(state_protocol_version);
// CRC calculated and verified
FSM_DECLARE_STATE(state_crc);
// Datagram Type processed
FSM_DECLARE_STATE(state_dgram_type);
// Destination nodes length processed
FSM_DECLARE_STATE(state_dst_len);
// Destination nodes data processed 
FSM_DECLARE_STATE(state_dst);
// Msg data length processed
FSM_DECLARE_STATE(state_data_len);
// Msg data processed
FSM_DECLARE_STATE(state_data);
// Cleanup and exit on error/completion
FSM_DECLARE_STATE(state_done);

FSM_STATE_TRANSITION(state_idle) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->transition_event, state_protocol_version);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_protocol_version) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->transition_event, state_crc);
  FSM_ADD_TRANSITION(store->repeat_event, state_protocol_version);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_crc) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->transition_event, state_dgram_type);
  FSM_ADD_TRANSITION(store->repeat_event, state_crc);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_dgram_type) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->transition_event, state_dst_len);
  FSM_ADD_TRANSITION(store->repeat_event, state_dgram_type);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_dst_len) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->transition_event, state_dst);
  FSM_ADD_TRANSITION(store->repeat_event, state_dst_len);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_dst) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->transition_event, state_data_len);
  FSM_ADD_TRANSITION(store->repeat_event, state_dst);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_data_len) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->transition_event, state_data);
  FSM_ADD_TRANSITION(store->repeat_event, state_data_len);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_data) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->transition_event, state_done);
  FSM_ADD_TRANSITION(store->repeat_event, state_data);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_done) {}

// RX Circular buffer functions
static void prv_init_rx_buffer(void) {
  s_rx_msg_buf.head = s_rx_msg_buf.buf;
  s_rx_msg_buf.tail = s_rx_msg_buf.head;
  s_data_write_count = 0;
}

static void prv_write_rx_buffer(uint8_t *data, size_t len) {
  // Catch buffer overflow as error
  if (s_data_write_count >= DATAGRAM_RX_BUFFER_LEN) {
    event_raise_no_data(s_store.error_event);
    LOG_WARN("RX MSG BUFFER OVERFLOW. EXITING WITH ERROR...");
  }
  if (s_rx_msg_buf.head - s_rx_msg_buf.buf > DATAGRAM_RX_BUFFER_LEN) {
    s_rx_msg_buf.head = s_rx_msg_buf.buf;
  }
  (s_rx_msg_buf.head)->len = len;
  memcpy(s_rx_msg_buf.head->data, data, len);
  s_rx_msg_buf.head++;
  s_data_write_count++;
}

static bool prv_read_rx_buffer(uint8_t **data, size_t *len) {
  // Check that data exists to be read
  if (s_data_write_count < 1) {
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
  uint8_t stream[MAX_CRC_STREAM_SIZE_BYTES];
  uint8_t *write = stream;
  CanDatagram *dgram = &s_store.dgram;

  crc32_init();
  // CRC for data and dst_nodes
  uint32_t dst_crc = crc32_arr(dgram->destination_nodes, dgram->destination_nodes_len);
  uint32_t data_crc = crc32_arr(dgram->data, dgram->data_len);

  // Copy metadata to stream, append dst and data crc
  memcpy(write, &dgram->dgram_type, DGRAM_TYPE_SIZE_BYTES);
  write += DGRAM_TYPE_SIZE_BYTES;
  memcpy(write, &dgram->destination_nodes_len, DEST_LEN_SIZE_BYTES);
  write += DEST_LEN_SIZE_BYTES;
  memcpy(write, &dst_crc, sizeof(uint32_t));
  write += sizeof(uint32_t);
  memcpy(write, &dgram->data_len, DATA_LEN_SIZE_BYTES);
  write += DATA_LEN_SIZE_BYTES;
  memcpy(write, &data_crc, sizeof(uint32_t));
  write += sizeof(uint32_t);

  // process final crc
  return crc32_arr(stream, (size_t)(write - stream));
}

// BEGIN FSM STATE FUNCTIONS
static void prv_process_protocol_version(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  // Verify start message received/sent
  if (s_store.start == false) {
    LOG_WARN("START MESSAGE NOT RECEIVED. EXITING WITH ERROR...\n");
    event_raise_no_data(store->error_event);
    return;
  }
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memcpy(s_can_buffer, &dgram->protocol_version, PROTOCOL_VERSION_SIZE_BYTES);
    s_store.tx_cb(s_can_buffer, PROTOCOL_VERSION_SIZE_BYTES, false);
    event_raise_no_data(store->transition_event);
    LOG_DEBUG("crc\n");
  } else {
    uint8_t *rx_data;
    size_t len;
    // Check if data has been written yet, if not, re-raise
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(store->repeat_event);
    } else {
      memcpy(&dgram->protocol_version, rx_data, PROTOCOL_VERSION_SIZE_BYTES);
      if (dgram->protocol_version != CAN_DATAGRAM_VERSION) {
        event_raise_no_data(store->error_event);
        return;
      }
      event_raise_no_data(store->transition_event);
      LOG_DEBUG("crc\n");
    }
  }
}

static void prv_process_crc(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memcpy(s_can_buffer, &dgram->crc, CRC_SIZE_BYTES);
    s_store.tx_cb(s_can_buffer, CRC_SIZE_BYTES, false);
    event_raise_no_data(store->transition_event);
    LOG_DEBUG("dgram_type\n");
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(store->repeat_event);
    } else {
      memcpy(&dgram->crc, rx_data, CRC_SIZE_BYTES);
      event_raise_no_data(store->transition_event);
      LOG_DEBUG("dgram_type\n");
    }
  }
}

static void prv_process_dgram_type(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memcpy(s_can_buffer, &dgram->dgram_type, DGRAM_TYPE_SIZE_BYTES);
    s_store.tx_cb(s_can_buffer, sizeof(dgram->dgram_type), false);
    event_raise_no_data(store->transition_event);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(store->repeat_event);
      LOG_DEBUG("dst_len\n");
    } else {
      memcpy(&dgram->dgram_type, rx_data, DGRAM_TYPE_SIZE_BYTES);
      event_raise_no_data(store->transition_event);
      LOG_DEBUG("dst_len\n");
    }
  }
}

static void prv_process_dst_len(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memcpy(s_can_buffer, &dgram->destination_nodes_len, DEST_LEN_SIZE_BYTES);
    s_store.tx_cb(s_can_buffer, sizeof(dgram->destination_nodes_len), false);
    event_raise_no_data(store->transition_event);
    LOG_DEBUG("dst\n");
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(store->repeat_event);
    } else {
      memcpy(&dgram->destination_nodes_len, rx_data, DEST_LEN_SIZE_BYTES);
      event_raise_no_data(store->transition_event);
      LOG_DEBUG("dst\n");
    }
  }
}

static void prv_process_dst(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  uint8_t dst_len = dgram->destination_nodes_len;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    uint8_t dst_bytes_to_send = 0;
    // TX max of 4 messages at a time
    for (int msg = 0; msg < MAX_CAN_TX; msg++) {
      dst_bytes_to_send = (dst_len - s_store.tx_bytes_sent < CAN_BUFFER_SIZE)
                              ? (dst_len - s_store.tx_bytes_sent)
                              : CAN_BUFFER_SIZE;
      memcpy(s_can_buffer, dgram->destination_nodes + s_store.tx_bytes_sent,
             (size_t)dst_bytes_to_send);
      s_store.tx_bytes_sent += dst_bytes_to_send;
      s_store.tx_cb(s_can_buffer, dst_bytes_to_send, false);
      if (s_store.tx_bytes_sent == dst_len) {
        LOG_DEBUG("data len\n");
        event_raise_no_data(store->transition_event);
        s_store.tx_bytes_sent = 0;
        return;
      }
    }
    event_raise_no_data(store->repeat_event);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(store->repeat_event);
    } else {
      memcpy(dgram->destination_nodes + s_store.rx_bytes_read, rx_data, len);
      s_store.rx_bytes_read += len;
      if (s_store.rx_bytes_read == dgram->destination_nodes_len) {
        LOG_DEBUG("data_len\n");
        s_store.rx_bytes_read = 0;
        event_raise_no_data(store->transition_event);
      } else {
        event_raise_no_data(store->repeat_event);
      }
    }
  }
}

static void prv_process_data_len(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    memcpy(s_can_buffer, &dgram->data_len, DATA_LEN_SIZE_BYTES);
    s_store.tx_cb(s_can_buffer, sizeof(dgram->data_len), false);
    event_raise_no_data(store->transition_event);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(store->repeat_event);
      LOG_DEBUG("data\n");
    } else {
      memcpy(&dgram->data_len, rx_data, DATA_LEN_SIZE_BYTES);
      event_raise_no_data(store->transition_event);
      LOG_DEBUG("data\n");
    }
  }
}

static void prv_process_data(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  uint16_t data_len = dgram->data_len;
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    uint8_t data_bytes_to_send = 0;
    // TX max of 4 messages at a time
    for (int msg = 0; msg < MAX_CAN_TX; msg++) {
      data_bytes_to_send = (data_len - s_store.tx_bytes_sent < CAN_BUFFER_SIZE)
                               ? (data_len - s_store.tx_bytes_sent)
                               : CAN_BUFFER_SIZE;
      memcpy(s_can_buffer, dgram->data + s_store.tx_bytes_sent, (size_t)data_bytes_to_send);
      s_store.tx_bytes_sent += data_bytes_to_send;
      s_store.tx_cb(s_can_buffer, data_bytes_to_send, false);
      if (s_store.tx_bytes_sent == data_len) {
        event_raise_no_data(store->transition_event);
        s_store.tx_bytes_sent = 0;
        LOG_DEBUG("datagram complete!\n");
        watchdog_cancel(&s_watchdog);
        return;
      }
    }
    event_raise_no_data(store->repeat_event);
  } else {
    uint8_t *rx_data;
    size_t len;
    if (!prv_read_rx_buffer(&rx_data, &len)) {
      event_raise_no_data(store->repeat_event);
    } else {
      memcpy(dgram->data + s_store.rx_bytes_read, rx_data, len);
      s_store.rx_bytes_read += len;
      if (s_store.rx_bytes_read >= dgram->data_len) {
        s_store.rx_bytes_read = 0;
        if (prv_can_datagram_compute_crc() == dgram->crc) {
          event_raise_no_data(store->transition_event);
          LOG_DEBUG("datagram complete!\n");
        } else {
          event_raise_no_data(store->error_event);
          LOG_WARN("CRC VALUE DID NOT MATCH. EXITING WITH ERROR...\n");
        }
      } else {
        event_raise_no_data(store->repeat_event);
      }
    }
  }
}

static void prv_process_done(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  if (e->id == store->transition_event) {
    store->status = DATAGRAM_STATUS_COMPLETE;
  } else {
    store->status = DATAGRAM_STATUS_ERROR;
  }
  watchdog_cancel(&s_watchdog);
}

static void prv_init_fsm(void *context) {
  fsm_init(&s_dgram_fsm, "can_dgram_fsm", &state_idle, context);
  fsm_state_init(state_protocol_version, prv_process_protocol_version);
  fsm_state_init(state_crc, prv_process_crc);
  fsm_state_init(state_dgram_type, prv_process_dgram_type);
  fsm_state_init(state_dst_len, prv_process_dst_len);
  fsm_state_init(state_dst, prv_process_dst);
  fsm_state_init(state_data_len, prv_process_data_len);
  fsm_state_init(state_data, prv_process_data);
  fsm_state_init(state_done, prv_process_done);
}

static void prv_rx_watchdog_expiry_cb(void *context) {
  event_raise_no_data(s_store.error_event);
  LOG_WARN("WATCHDOG EXPIRED. EXITING WITH ERROR...\n");
}

StatusCode can_datagram_init(CanDatagramSettings *settings) {
  if (settings->mode >= NUM_CAN_DATAGRAM_MODES) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (settings->data_len > MAX_DATA_SIZE_BYTES) {
    return STATUS_CODE_INVALID_ARGS;
  }
  // Populate storage
  s_store.mode = settings->mode;
  s_store.status = DATAGRAM_STATUS_OK;
  s_store.start = false;
  s_store.transition_event = settings->transition_event;
  s_store.repeat_event = settings->repeat_event;
  s_store.error_event = settings->error_event;

  // Datagram initialization
  CanDatagram *dgram = &s_store.dgram;
  memset(&s_store.dgram, 0, sizeof(s_store.dgram));
  dgram->dgram_type = settings->dgram_type;
  dgram->destination_nodes_len = settings->destination_nodes_len;
  dgram->data_len = settings->data_len;
  dgram->destination_nodes = settings->destination_nodes;
  dgram->data = settings->data;
  dgram->crc = prv_can_datagram_compute_crc();

  // Mode specific setup
  if (s_store.mode == CAN_DATAGRAM_MODE_TX) {
    if (settings->tx_cb == NULL) {
      return STATUS_CODE_INVALID_ARGS;
    }
    s_store.tx_cb = settings->tx_cb;
    dgram->protocol_version = CAN_DATAGRAM_VERSION;
  } else {
    // Set up rx circular buffer, zero data buffers
    prv_init_rx_buffer();
    memset(dgram->destination_nodes, 0, dgram->destination_nodes_len);
    memset(dgram->data, 0, dgram->data_len);
  }
  prv_init_fsm((void *)&s_store);
  return STATUS_CODE_OK;
}

StatusCode can_datagram_start_tx(uint8_t *init_data, size_t len) {
  // do error checking here before commencing
  if (s_store.mode == CAN_DATAGRAM_MODE_TX && len <= CAN_BUFFER_SIZE) {
    s_store.start = true;
    s_store.tx_cb(init_data, len, true);
    event_raise_no_data(s_store.transition_event);
    LOG_DEBUG("protocol_version\n");
  } else {
    return STATUS_CODE_INVALID_ARGS;
  }
  return STATUS_CODE_OK;
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
    event_raise_no_data(s_store.transition_event);
    LOG_DEBUG("protocol_version\n");
    watchdog_start(&s_watchdog, RX_WATCHDOG_TIMEOUT_MS, prv_rx_watchdog_expiry_cb, NULL);
    return STATUS_CODE_OK;
  } else {
    // If start message not received, do not rx data
    if (s_store.start == false) {
      event_raise_no_data(s_store.error_event);
      LOG_WARN("NO START MESSAGE RECEIVED. EXITING WITH ERROR...\n");
    }
  }
  watchdog_kick(&s_watchdog);
  prv_write_rx_buffer(data, len);
  return STATUS_CODE_OK;
}

CanDatagramStatus can_datagram_get_status(void) {
  return s_store.status;
}

bool can_datagram_process_event(Event *e) {
  return fsm_process_event(&s_dgram_fsm, e);
}

CanDatagram *can_datagram_get_datagram(void) {
  return &s_store.dgram;
}
