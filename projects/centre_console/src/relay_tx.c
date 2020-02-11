#include "relay_tx.h"
#include "can_transmit.h"

#define NUM_TX_RETRIES 5

StatusCode relay_tx_init(RelayTxStorage *storage) {
  CanTxRetryWrapperSettings retry_settings = {
    .retries =  NUM_TX_RETRIES
  };
  can_tx_retry_wrapper_init(&storage->can_retry_wrapper_storage, &retry_settings);
  return STATUS_CODE_OK;
}

static void prv_tx_relay_state(CanAckRequest *ack_ptr, void *context) {
  RelayTxStorage *storage = (RelayTxStorage *) context;
  uint16_t relay_mask = 1 << storage->relay_id;
  uint16_t relay_state = storage->relay_state << storage->relay_id;
  CAN_TRANSMIT_SET_RELAY_STATES(ack_ptr, relay_mask, relay_state);
}

StatusCode relay_tx_relay_state(RelayTxStorage *storage, RetryTxRequest *request,
                                EERelayId id, EERelayState state) {
  if (id >= NUM_EE_RELAY_IDS || state >= NUM_EE_RELAY_STATES) {
    return STATUS_CODE_INVALID_ARGS;
  }
  CanTxRetryWrapperRequest retry_wrapper_request = {
    .retry_request = *request,
    .ack_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_BMS_CARRIER),
    .tx_callback = prv_tx_relay_state,
  };
  can_tx_retry_send(&storage->can_retry_wrapper_storage, &retry_wrapper_request);
  return STATUS_CODE_OK;
}
