#include "can_datagram.h"

#include <stdlib.h>
#include <string.h>

#include "crc32.h"
#include "fsm.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "watchdog.h"
#include "fifo.h"

#define PROTOCOL_VERSION_SIZE_BYTES 1
#define CRC_SIZE_BYTES 4
#define DGRAM_TYPE_SIZE_BYTES 1
#define DEST_LEN_SIZE_BYTES 1
#define MAX_DEST_NODES_SIZE_BYTES 255
#define DATA_LEN_SIZE_BYTES 2
#define MAX_DATA_SIZE_BYTES 2048
#define MAX_CAN_TX 4

#define DATAGRAM_BUFFER_LEN 2048 

#define RX_WATCHDOG_TIMEOUT_MS 25
#define CAN_BUFFER_SIZE 8

typedef struct CanDatagramStorage {
  CanDatagram dgram;
  CanDatagramTxCb tx_cb;
  CanDatagramStatus status;
  CanDatagramRxConfig *rx_info;

  EventId tx_event;
  EventId rx_event;
  EventId repeat_event;
  EventId error_event;

  size_t tx_bytes_sent;
  bool rx_listener_enabled;
  uint8_t node_id;
} CanDatagramStorage;

static CanDatagramStorage s_store;
static Fsm s_dgram_fsm;
static WatchdogStorage s_watchdog;

static Fifo s_fifo;
static uint8_t s_buffer[DATAGRAM_BUFFER_LEN];
static uint8_t s_can_buffer[CAN_BUFFER_SIZE];

// Forward declare FSM states and transitions
// Each state can transition to next on completion of processing
// or return to itself if data has not been received or has yet
// to be processed

// Idle state
FSM_DECLARE_STATE(state_idle);

// Protocol Version processed
FSM_DECLARE_STATE(state_tx_protocol_version);
// CRC calculated and verified
FSM_DECLARE_STATE(state_tx_crc);
// Datagram Type processed
FSM_DECLARE_STATE(state_tx_dgram_type);
// Destination nodes length processed
FSM_DECLARE_STATE(state_tx_dst_len);
// Destination nodes data processed 
FSM_DECLARE_STATE(state_tx_dst);
// Msg data length processed
FSM_DECLARE_STATE(state_tx_data_len);
// Msg data processed
FSM_DECLARE_STATE(state_tx_data);
// Protocol Version processed

FSM_DECLARE_STATE(state_rx_protocol_version);
// CRC calculated and verified
FSM_DECLARE_STATE(state_rx_crc);
// Datagram Type processed
FSM_DECLARE_STATE(state_rx_dgram_type);
// Destination nodes length processed
FSM_DECLARE_STATE(state_rx_dst_len);
// Destination nodes data processed 
FSM_DECLARE_STATE(state_rx_dst);
// Msg data length processed
FSM_DECLARE_STATE(state_rx_data_len);
// Msg data processed
FSM_DECLARE_STATE(state_rx_data);

// Cleanup and exit on error/completion
FSM_DECLARE_STATE(state_done);

FSM_STATE_TRANSITION(state_idle) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->tx_event, state_tx_protocol_version);
  FSM_ADD_TRANSITION(store->rx_event, state_rx_protocol_version);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_tx_protocol_version) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->tx_event, state_tx_crc);
  FSM_ADD_TRANSITION(store->repeat_event, state_tx_protocol_version);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_tx_crc) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->tx_event, state_tx_dgram_type);
  FSM_ADD_TRANSITION(store->repeat_event, state_tx_crc);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_tx_dgram_type) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->tx_event, state_tx_dst_len);
  FSM_ADD_TRANSITION(store->repeat_event, state_tx_dgram_type);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_tx_dst_len) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->tx_event, state_tx_dst);
  FSM_ADD_TRANSITION(store->repeat_event, state_tx_dst_len);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_tx_dst) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->tx_event, state_tx_data_len);
  FSM_ADD_TRANSITION(store->repeat_event, state_tx_dst);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_tx_data_len) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->tx_event, state_tx_data);
  FSM_ADD_TRANSITION(store->repeat_event, state_tx_data_len);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_tx_data) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->tx_event, state_done);
  FSM_ADD_TRANSITION(store->repeat_event, state_tx_data);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_rx_protocol_version) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->rx_event, state_rx_crc);
  FSM_ADD_TRANSITION(store->repeat_event, state_rx_protocol_version);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_rx_crc) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->rx_event, state_rx_dgram_type);
  FSM_ADD_TRANSITION(store->repeat_event, state_rx_crc);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_rx_dgram_type) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->rx_event, state_rx_dst_len);
  FSM_ADD_TRANSITION(store->repeat_event, state_rx_dgram_type);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_rx_dst_len) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->rx_event, state_rx_dst);
  FSM_ADD_TRANSITION(store->repeat_event, state_rx_dst_len);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_rx_dst) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->rx_event, state_rx_data_len);
  FSM_ADD_TRANSITION(store->repeat_event, state_rx_dst);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_rx_data_len) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->rx_event, state_rx_data);
  FSM_ADD_TRANSITION(store->repeat_event, state_rx_data_len);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_rx_data) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->rx_event, state_done);
  FSM_ADD_TRANSITION(store->repeat_event, state_rx_data);
  FSM_ADD_TRANSITION(store->error_event, state_done);
}

FSM_STATE_TRANSITION(state_done) {
  CanDatagramStorage *store = fsm->context;
  FSM_ADD_TRANSITION(store->rx_event, state_idle);
  FSM_ADD_TRANSITION(store->tx_event, state_idle);
  FSM_ADD_TRANSITION(store->error_event, state_idle);
}

//  Write max 8 bytes to fifo on datagram frame rx
static void prv_write_rx_buffer(uint8_t *data, size_t len) {
  int ret = fifo_push_arr(&s_fifo, data, len);
    LOG_WARN("RX WRITE CALLED\n");
  // Catch buffer overflow as error
  if (ret == STATUS_CODE_RESOURCE_EXHAUSTED) {
    LOG_WARN("RX MSG BUFFER OVERFLOW. EXITING WITH ERROR...\n");
    event_raise(s_store.error_event, HARD_ERROR);
  }
}

// Read array of max length 2048 direct from fifo
static bool prv_read_rx_buffer(uint8_t *data, size_t len) {
  // Check that data exists to be read
  LOG_WARN("RX READ CALLED\n");
  int ret = fifo_pop_arr(&s_fifo, data, len);
  if (ret == STATUS_CODE_RESOURCE_EXHAUSTED) {
    return false;
  }
  return true;
}

// Functions used to write and read to fifo
static void prv_flush_tx_buffer(void) {
  size_t tx_len = fifo_size(&s_fifo) < CAN_BUFFER_SIZE ? fifo_size(&s_fifo) : CAN_BUFFER_SIZE;
  for (size_t i = 0; i < tx_len; i++) {
	fifo_pop(&s_fifo, &s_can_buffer[i]);
  }
  fifo_pop_arr(&s_fifo, s_can_buffer, tx_len);
  s_store.tx_cb(s_can_buffer, tx_len, false);
}

// Functions used to write and read to fifo
static void prv_write_tx_buffer(uint8_t *data, size_t len) {
  fifo_push_arr(&s_fifo, data, len);
  // If 8 bytes of data available, transmit
  // Final flush called from state_done
  if (fifo_size(&s_fifo) >= CAN_BUFFER_SIZE) {
    prv_flush_tx_buffer();
  }
}


static uint32_t prv_can_datagram_compute_crc(void) {
  uint32_t crc;
  uint8_t crc_stream_size = DGRAM_TYPE_SIZE_BYTES + DEST_LEN_SIZE_BYTES 
     + DATA_LEN_SIZE_BYTES + 2*sizeof(uint32_t);

  uint8_t stream[crc_stream_size];
  uint8_t *write = stream;
  CanDatagram *dgram = &s_store.dgram;

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

  // process final crcs
  return crc32_arr(stream, (size_t)(write - stream));
}

// BEGIN FSM STATE FUNCTIONS
static void prv_process_protocol_version_tx(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;

  memcpy(s_can_buffer, &dgram->protocol_version, PROTOCOL_VERSION_SIZE_BYTES);
  prv_write_tx_buffer(s_can_buffer, PROTOCOL_VERSION_SIZE_BYTES);
  event_raise_no_data(store->tx_event);
  LOG_DEBUG("crc\n");
}

static void prv_process_crc_tx(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  memcpy(s_can_buffer, &dgram->crc, CRC_SIZE_BYTES);
  prv_write_tx_buffer(s_can_buffer, CRC_SIZE_BYTES);
  event_raise_no_data(store->tx_event);
  LOG_DEBUG("dgram_type\n");
}

static void prv_process_dgram_type_tx(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  memcpy(s_can_buffer, &dgram->dgram_type, DGRAM_TYPE_SIZE_BYTES);
  prv_write_tx_buffer(s_can_buffer, sizeof(dgram->dgram_type));
  event_raise_no_data(store->tx_event);
}

static void prv_process_dst_len_tx(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  memcpy(s_can_buffer, &dgram->destination_nodes_len, DEST_LEN_SIZE_BYTES);
  prv_write_tx_buffer(s_can_buffer, sizeof(dgram->destination_nodes_len));
  event_raise_no_data(store->tx_event);
  LOG_DEBUG("dst\n");
}

static void prv_process_dst_tx(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  uint8_t dst_len = dgram->destination_nodes_len;
  uint8_t dst_bytes_to_send = 0;
  // TX max of 4 messages at a time
  for (int msg = 0; msg < MAX_CAN_TX; msg++) {
    dst_bytes_to_send = (dst_len - s_store.tx_bytes_sent < CAN_BUFFER_SIZE)
                            ? (dst_len - s_store.tx_bytes_sent)
                            : CAN_BUFFER_SIZE;
    memcpy(s_can_buffer, dgram->destination_nodes + s_store.tx_bytes_sent,
           (size_t)dst_bytes_to_send);
    s_store.tx_bytes_sent += dst_bytes_to_send;
    prv_write_tx_buffer(s_can_buffer, dst_bytes_to_send);
    if (s_store.tx_bytes_sent == dst_len) {
      LOG_DEBUG("data len\n");
      event_raise_no_data(store->tx_event);
      s_store.tx_bytes_sent = 0;
      return;
    }
  }
  event_raise_no_data(store->repeat_event);
}

static void prv_process_data_len_tx(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  memcpy(s_can_buffer, &dgram->data_len, DATA_LEN_SIZE_BYTES);
  prv_write_tx_buffer(s_can_buffer, sizeof(dgram->data_len));
  event_raise_no_data(store->tx_event);
  LOG_DEBUG("data\n");
}

static void prv_process_data_tx(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  uint16_t data_len = dgram->data_len;
  uint8_t data_bytes_to_send = 0;
  // Push max of 4 messages, 8 bytes at a time to fifo for tx
  for (int msg = 0; msg < MAX_CAN_TX; msg++) {
    data_bytes_to_send = (data_len - s_store.tx_bytes_sent < CAN_BUFFER_SIZE)
                             ? (data_len - s_store.tx_bytes_sent)
                             : CAN_BUFFER_SIZE;
    memcpy(s_can_buffer, dgram->data + s_store.tx_bytes_sent, (size_t)data_bytes_to_send);
    s_store.tx_bytes_sent += data_bytes_to_send;
    prv_write_tx_buffer(s_can_buffer, data_bytes_to_send);
    if (s_store.tx_bytes_sent == data_len) {
      //tx any final data
      if (fifo_size(&s_fifo) > 0) {
      	prv_flush_tx_buffer();
      }
      event_raise_no_data(store->tx_event);
      s_store.tx_bytes_sent = 0;
      LOG_DEBUG("datagram complete!\n");
      return;
    }
  }
  event_raise_no_data(store->repeat_event);
}

static void prv_process_protocol_version_rx(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;

  // Check if data has been written yet, if not, re-raise
  if (!prv_read_rx_buffer(&dgram->protocol_version, PROTOCOL_VERSION_SIZE_BYTES)) {
    event_raise_no_data(store->repeat_event);
  } else {
    if (dgram->protocol_version != CAN_DATAGRAM_VERSION) {
      // Soft error, will return us to state idle
      LOG_WARN("DIFFERENT PROTOCOL VERSION, MSG IGNORED\n");
      event_raise(store->error_event, SOFT_ERROR);
      return;
    }
    event_raise_no_data(store->rx_event);
    LOG_DEBUG("crc\n");
  }
}

static void prv_process_crc_rx(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  if (!prv_read_rx_buffer((uint8_t*)&dgram->crc, CRC_SIZE_BYTES)) {
    event_raise_no_data(store->repeat_event);
  } else {
    event_raise_no_data(store->rx_event);
    LOG_DEBUG("dgram_type\n");
  }
}

static void prv_process_dgram_type_rx(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  if (!prv_read_rx_buffer(&dgram->dgram_type, DGRAM_TYPE_SIZE_BYTES)) {
    event_raise_no_data(store->repeat_event);
  } else {
    event_raise_no_data(store->rx_event);
    LOG_DEBUG("dst_len\n");
  }
}
 

static void prv_process_dst_len_rx(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  if (!prv_read_rx_buffer((uint8_t*)&dgram->destination_nodes_len, DEST_LEN_SIZE_BYTES)) {
    event_raise_no_data(store->repeat_event);
  } else {
    event_raise_no_data(store->rx_event);
    LOG_DEBUG("dst\n");
  }
}


static void prv_process_dst_rx(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  if (!prv_read_rx_buffer(dgram->destination_nodes, dgram->destination_nodes_len)) {
    event_raise_no_data(store->repeat_event);
  } else {
      // Check to see if current ID in list, otherwise ignore msg
      bool found_id = false;
      for(uint8_t id = 0; id < dgram->destination_nodes_len; id++) {
        if (dgram->destination_nodes[id] == s_store.node_id) {
          found_id = true;
	}
      }
      if(!found_id) {
        LOG_WARN("CURRENT NODE NOT REQUESTED, MSG IGNORED\n");
        event_raise(store->error_event, SOFT_ERROR);
      } else {
      	LOG_DEBUG("data_len\n");
      	event_raise_no_data(store->rx_event);
      }
  }   
} 


static void prv_process_data_len_rx(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  if (!prv_read_rx_buffer((uint8_t*)&dgram->data_len, DATA_LEN_SIZE_BYTES)) {
    event_raise_no_data(store->repeat_event);
  } else {
    event_raise_no_data(store->rx_event);
    LOG_DEBUG("data\n");
  }
}

static void prv_process_data_rx(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  CanDatagram *dgram = &store->dgram;
  if (!prv_read_rx_buffer(dgram->data, dgram->data_len)) {
    event_raise_no_data(store->repeat_event);
  } else {
    if (prv_can_datagram_compute_crc() == dgram->crc) {
      event_raise_no_data(store->rx_event);
      LOG_DEBUG("datagram complete!\n");
    } else {
      LOG_WARN("CRC VALUE DID NOT MATCH. EXITING WITH ERROR...\n");
      event_raise(store->error_event, HARD_ERROR);
    }
  }
}
 
static void prv_process_done(Fsm *fsm, const Event *e, void *context) {
  CanDatagramStorage *store = context;
  // Handle tx specific cleanup
  if (e->id == store->tx_event) {
    // Clear last tx data
    prv_flush_tx_buffer();
    store->status = DATAGRAM_STATUS_TX_COMPLETE;
  // Handle rx specific cleanup
  } else if (e->id == store->rx_event) {
    // Write info to passed rx config
    s_store.rx_info->dgram_type = s_store.dgram.dgram_type;
    s_store.rx_info->destination_nodes_len = s_store.dgram.destination_nodes_len;
    s_store.rx_info->data_len = s_store.dgram.data_len;
    store->rx_listener_enabled = false;
    store->status = DATAGRAM_STATUS_RX_COMPLETE;
  } else {
    // Handling errors
    // Empty Fifo
    uint8_t out;
    while(fifo_size(&s_fifo) > 0) {
      fifo_pop(&s_fifo, &out);
    }
    if (e->data == HARD_ERROR) {
      store->rx_listener_enabled = false;
      store->status = DATAGRAM_STATUS_ERROR;
    } // Soft errors just return to start and continue
    event_raise(s_store.error_event, e->data);
  }
  watchdog_cancel(&s_watchdog);
}

static void prv_init_fsm(void *context) {
  fsm_init(&s_dgram_fsm, "can_dgram_fsm", &state_idle, context);

  fsm_state_init(state_tx_protocol_version, prv_process_protocol_version_tx);
  fsm_state_init(state_tx_crc, prv_process_crc_tx);
  fsm_state_init(state_tx_dgram_type, prv_process_dgram_type_tx);
  fsm_state_init(state_tx_dst_len, prv_process_dst_len_tx);
  fsm_state_init(state_tx_dst, prv_process_dst_tx);
  fsm_state_init(state_tx_data_len, prv_process_data_len_tx);
  fsm_state_init(state_tx_data, prv_process_data_tx);

  fsm_state_init(state_rx_protocol_version, prv_process_protocol_version_rx);
  fsm_state_init(state_rx_crc, prv_process_crc_rx);
  fsm_state_init(state_rx_dgram_type, prv_process_dgram_type_rx);
  fsm_state_init(state_rx_dst_len, prv_process_dst_len_rx);
  fsm_state_init(state_rx_dst, prv_process_dst_rx);
  fsm_state_init(state_rx_data_len, prv_process_data_len_rx);
  fsm_state_init(state_rx_data, prv_process_data_rx);

  fsm_state_init(state_done, prv_process_done);
}

static void prv_rx_watchdog_expiry_cb(void *context) {
  LOG_WARN("WATCHDOG EXPIRED. EXITING WITH ERROR...\n");
  event_raise(s_store.error_event, HARD_ERROR);
}

StatusCode can_datagram_init(CanDatagramSettings *settings) {

  // Populate storage
  s_store.tx_event = settings->tx_event;
  s_store.rx_event = settings->rx_event;
  s_store.repeat_event = settings->repeat_event;
  s_store.error_event = settings->error_event;


  fifo_init(&s_fifo, s_buffer);
  prv_init_fsm((void *)&s_store);
  s_store.status = DATAGRAM_STATUS_IDLE;

  return STATUS_CODE_OK;
}

StatusCode can_datagram_rx(uint8_t *data, size_t len, bool start_message) {
  if (len > CAN_BUFFER_SIZE) {
    return STATUS_CODE_OUT_OF_RANGE;
  }

  // Only run if start_listener called 
  if (!s_store.rx_listener_enabled) {
	  return STATUS_CODE_UNINITIALIZED;
  }
  
  if (start_message) {
    // Only process start messages if rx is not currently active
    if (s_store.status != DATAGRAM_STATUS_ACTIVE) {
      // Start frame rcv'd -> status now active
      // Commence rx side of state machine
      s_store.status = DATAGRAM_STATUS_ACTIVE;
      event_raise_no_data(s_store.rx_event);
      LOG_DEBUG("protocol_version\n");
      watchdog_start(&s_watchdog, RX_WATCHDOG_TIMEOUT_MS, prv_rx_watchdog_expiry_cb, NULL);
      return STATUS_CODE_OK;
    } else {
      return STATUS_CODE_UNREACHABLE;
    }
  } else if (s_store.status != DATAGRAM_STATUS_ACTIVE) {
    // If start message not received, do not rx data
    return STATUS_CODE_UNINITIALIZED;
  }

  // Rx normal data
  watchdog_kick(&s_watchdog);
  prv_write_rx_buffer(data, len);
  return STATUS_CODE_OK;
}

StatusCode can_datagram_start_listener(CanDatagramRxConfig *config) {
  // Set up rx fifo for rxed datagram messages, zero data buffers
  if (s_store.status == DATAGRAM_STATUS_ACTIVE) {
    return STATUS_CODE_UNREACHABLE;
  }
  // Set up data buffers
  CanDatagram *dgram = &s_store.dgram;
  dgram->data = config->data;
  dgram->destination_nodes = config->destination_nodes;
  memset(dgram->destination_nodes, 0, dgram->destination_nodes_len);
  memset(dgram->data, 0, dgram->data_len);
  
  // Store config * to be filled out after rx complete
  s_store.rx_info = config;
  
  // Store current node id
  s_store.node_id = config->node_id;

  s_store.rx_listener_enabled = true;
  return STATUS_CODE_OK;
}

StatusCode can_datagram_start_tx(CanDatagramTxConfig *config) {
  // Verify datagram in idle state
  if (s_store.status == DATAGRAM_STATUS_ACTIVE) {
    return STATUS_CODE_UNREACHABLE;
  }
  // Do error checking before commencing
  if (config->tx_cb == NULL) {
      return STATUS_CODE_INVALID_ARGS;
  }
  if (config->data_len > MAX_DATA_SIZE_BYTES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // Datagram initialization
  CanDatagram *dgram = &s_store.dgram;

  dgram->dgram_type = config->dgram_type;
  dgram->destination_nodes_len = config->destination_nodes_len;
  dgram->data_len = config->data_len;
  dgram->destination_nodes = config->destination_nodes;
  dgram->data = config->data;
  dgram->crc = prv_can_datagram_compute_crc();

  s_store.tx_cb = config->tx_cb;
  s_store.tx_cb(NULL, 0, true);

  event_raise_no_data(s_store.tx_event);
  LOG_DEBUG("protocol_version\n");
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

