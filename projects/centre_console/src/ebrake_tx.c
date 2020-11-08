#include "ebrake_tx.h"
#include "can_transmit.h"
#include "log.h"

static void prv_tx_ebrake_state(CanAckRequest *ack_ptr, void *context) {
  EbrakeTxStorage *storage = (EbrakeTxStorage *)context;
  CAN_TRANSMIT_SET_EBRAKE_STATE(ack_ptr, storage->state);
}

StatusCode ebrake_tx_init(EbrakeTxStorage *storage) {
  CanTxRetryWrapperSettings retry_settings = { .retries = NUM_EBRAKE_TX_RETRIES };
  storage->current_state = EE_EBRAKE_STATE_RELEASED;
  status_ok_or_return(
      can_tx_retry_wrapper_init(&storage->can_retry_wrapper_storage, &retry_settings));
  return STATUS_CODE_OK;
}

EEEbrakeState get_current_state(EbrakeTxStorage *storage) {
  return storage->current_state;
}

void prv_success_ebrake_tx_callback(void *context) {
  LOG_DEBUG("ebrake set success\n");
  EbrakeTxStorage *storage = (EbrakeTxStorage *)context;
  storage->current_state = storage->state;
}

StatusCode ebrake_tx_brake_state(EbrakeTxStorage *storage, RetryTxRequest *request,
                                 EEEbrakeState state) {
  if (state >= NUM_EE_EBRAKE_STATES) {
    return STATUS_CODE_INVALID_ARGS;
  }
  storage->state = state;
  CanTxRetryWrapperRequest retry_wrapper_request = {
    .retry_request = *request,
    // .ack_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT),
    .ack_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_BABYDRIVER),
    .tx_callback = prv_tx_ebrake_state,
    .tx_callback_context = storage
  };
  status_ok_or_return(can_tx_retry_wrapper_register_success_callback(
      &storage->can_retry_wrapper_storage, prv_success_ebrake_tx_callback, storage));
  can_tx_retry_send(&storage->can_retry_wrapper_storage, &retry_wrapper_request);
  return STATUS_CODE_OK;
}
