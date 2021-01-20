#include "mcp3427_adc.h"
#include "fsm.h"
#include "log.h"
#include "mcp3427_adc_defs.h"
#include "soft_timer.h"

// x86 emulation of STM32 MCP3427 implementation.
// We emulate the FSM structure and return a fixed value for both channels.
// For a more advanced simulation we could probabilistically call the fault callback, but that would
// be hard to unit test.

#define FIXED_RESULT 20

#define MCP3427_FSM_NAME "MCP3427 FSM on x86"

#define NUM_MCP3427_CHIP_IDS (1 << 4)

#ifdef MPXE
#include "mcp3427.pb-c.h"
#include "store.h"
#include "stores.pb-c.h"

static MxMcp3427Store s_store = MX_MCP3427_STORE__INIT;

static void update_store(ProtobufCBinaryData msg_buf, ProtobufCBinaryData mask_buf) {
  MxMcp3427Store *msg = mx_mcp3427_store__unpack(NULL, msg_buf.len, msg_buf.data);
  MxMcp3427Store *mask = mx_mcp3427_store__unpack(NULL, mask_buf.len, mask_buf.data);

  for (uint8_t i = 0; i < NUM_MCP3427_CHANNELS; i++) {
    if (mask->readings[i] != 0) {
      s_store.readings[i] = msg->readings[i];
    }
  }

  mx_mcp3427_store__free_unpacked(msg, NULL);
  mx_mcp3427_store__free_unpacked(mask, NULL);
  store_export(MX_STORE_TYPE__MCP3427, &s_store, NULL);
}

static void prv_init_store(void) {
  store_config();
  StoreFuncs funcs = {
    (GetPackedSizeFunc)mx_mcp3427_store__get_packed_size,
    (PackFunc)mx_mcp3427_store__pack,
    (UnpackFunc)mx_mcp3427_store__unpack,
    (FreeUnpackedFunc)mx_mcp3427_store__free_unpacked,
    (UpdateStoreFunc)update_store,
  };
  s_store.n_readings = NUM_MCP3427_CHANNELS;
  s_store.readings = malloc(NUM_MCP3427_CHANNELS * sizeof(int32_t));

  store_register(MX_STORE_TYPE__MCP3427, funcs, &s_store, NULL);
}
#endif

// A lookup table of MCP3427 chip IDs (see |prv_get_chip_identifier|) to their storages,
// used to automagically direct events to the correct storage in |mcp3427_process_event|.
// This saves having to pass each event to every MCP3427, which is more of a concern on STM32,
// but we emulate STM32 behaviour nonetheless.
static Mcp3427Storage *s_id_to_storage_cache[NUM_MCP3427_CHIP_IDS] = { 0 };

FSM_DECLARE_STATE(channel_1_trigger);
FSM_DECLARE_STATE(channel_1_readback);
FSM_DECLARE_STATE(channel_2_trigger);
FSM_DECLARE_STATE(channel_2_readback);

FSM_STATE_TRANSITION(channel_1_trigger) {
  Mcp3427Storage *storage = fsm->context;
  FSM_ADD_TRANSITION(storage->data_ready_event, channel_1_readback);
}

FSM_STATE_TRANSITION(channel_1_readback) {
  Mcp3427Storage *storage = fsm->context;
  FSM_ADD_TRANSITION(storage->data_trigger_event, channel_2_trigger);
}

FSM_STATE_TRANSITION(channel_2_trigger) {
  Mcp3427Storage *storage = fsm->context;
  FSM_ADD_TRANSITION(storage->data_ready_event, channel_2_readback);
}

FSM_STATE_TRANSITION(channel_2_readback) {
  Mcp3427Storage *storage = fsm->context;
  FSM_ADD_TRANSITION(storage->data_trigger_event, channel_1_trigger);
}

static uint8_t prv_get_chip_identifier(Mcp3427Storage *storage) {
  // Used to gate events we raised to only this MCP3427.
  // ID is 4 bits, with the 3-bit base address on STM32 as the most significant bits and the I2C
  // port encoded as the LSB.
  return (storage->addr << 1) | (storage->port == I2C_PORT_1 ? 0 : 1);
}

static Mcp3427Channel other_channel(Mcp3427Channel channel) {
  return channel == MCP3427_CHANNEL_1 ? MCP3427_CHANNEL_2 : MCP3427_CHANNEL_1;
}

static void prv_raise_ready(SoftTimerId timer_id, void *context) {
  Mcp3427Storage *storage = (Mcp3427Storage *)context;
  event_raise(storage->data_ready_event, prv_get_chip_identifier(storage));
}

static void prv_channel_ready(struct Fsm *fsm, const Event *e, void *context) {
  Mcp3427Storage *storage = (Mcp3427Storage *)context;

  if (storage->current_channel == MCP3427_CHANNEL_2 && storage->callback != NULL) {
#ifdef MPXE
    s_store.readings[storage->current_channel] = FIXED_RESULT;
    if (s_store.fault_flag) {
      storage->fault_callback(storage->fault_context);
    }
#endif

    // We've "read" from both of the channels.
    storage->callback(FIXED_RESULT, FIXED_RESULT, storage->context);
  }

  event_raise(storage->data_trigger_event, prv_get_chip_identifier(storage));
}

// Schedule a data ready event to be raised.
static void prv_channel_trigger(struct Fsm *fsm, const Event *e, void *context) {
  Mcp3427Storage *storage = (Mcp3427Storage *)context;
  storage->current_channel = other_channel(storage->current_channel);
  soft_timer_start_millis(MCP3427_MAX_CONV_TIME_MS, prv_raise_ready, context, NULL);
}

// Address lookup table from STM32.
static uint8_t s_addr_lookup[NUM_MCP3427_PIN_STATES][NUM_MCP3427_PIN_STATES] = {
  { 0x0, 0x1, 0x2 },
  { 0x3, 0x0, 0x7 },
  { 0x4, 0x5, 0x6 },
};

StatusCode mcp3427_init(Mcp3427Storage *storage, Mcp3427Settings *settings) {
#ifdef MPXE
  prv_init_store();
#endif

  if (storage == NULL || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  storage->data_ready_event = settings->adc_data_ready_event;
  storage->data_trigger_event = settings->adc_data_trigger_event;

  // emulate the address and port from stm32 since it's used for the identifier
  storage->port = settings->port;
  storage->addr = s_addr_lookup[settings->addr_pin_0][settings->addr_pin_1];

  // Cache the storage for lookup in |mcp3427_process_event|
  s_id_to_storage_cache[prv_get_chip_identifier(storage)] = storage;

  fsm_state_init(channel_1_trigger, prv_channel_trigger);
  fsm_state_init(channel_1_readback, prv_channel_ready);
  fsm_state_init(channel_2_trigger, prv_channel_trigger);
  fsm_state_init(channel_2_readback, prv_channel_ready);

  // start the state machine ready to transition to channel_1_trigger
  storage->current_channel = MCP3427_CHANNEL_2;
  fsm_init(&storage->fsm, MCP3427_FSM_NAME, &channel_2_readback, storage);

  return STATUS_CODE_OK;
}

StatusCode mcp3427_register_callback(Mcp3427Storage *storage, Mcp3427Callback callback,
                                     void *context) {
  if (storage == NULL || callback == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  storage->callback = callback;
  storage->context = context;
  return STATUS_CODE_OK;
}

StatusCode mcp3427_register_fault_callback(Mcp3427Storage *storage, Mcp3427FaultCallback callback,
                                           void *context) {
  if (storage == NULL || callback == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  // We'll still set it even though it's never called on x86.
  storage->fault_callback = callback;
  storage->fault_context = context;
  return STATUS_CODE_OK;
}

StatusCode mcp3427_start(Mcp3427Storage *storage) {
  return event_raise(storage->data_trigger_event, prv_get_chip_identifier(storage));
}

StatusCode mcp3427_process_event(Event *e) {
  if (e == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (e->data >= NUM_MCP3427_CHIP_IDS) {
    // not for us
    return STATUS_CODE_OK;
  }

  // look up which storage to use
  Mcp3427Storage *storage = s_id_to_storage_cache[e->data];
  if (storage == NULL) {
    // also not for us
    return STATUS_CODE_OK;
  }

  // process the event for that storage
  fsm_process_event(&storage->fsm, e);
  return STATUS_CODE_OK;
}
