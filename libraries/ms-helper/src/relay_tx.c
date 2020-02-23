#include "relay_tx.h"
#include "can_transmit.h"
#include "can_pack.h"

StatusCode relay_tx_init(RelayTxStorage *storage) {
  CanTxRetryWrapperSettings retry_settings = { .retries = NUM_RELAY_TX_RETRIES };
  status_ok_or_return(can_tx_retry_wrapper_init(&storage->can_retry_wrapper_storage, &retry_settings));
  return STATUS_CODE_OK;
}

static void prv_tx_relay_state(CanAckRequest *ack_ptr, void *context) {
  RelayTxStorage *storage = (RelayTxStorage *)context;
  uint16_t relay_mask = 1 << storage->relay_id;
  uint16_t relay_state = storage->relay_state << storage->relay_id;
  CAN_TRANSMIT_SET_RELAY_STATES(ack_ptr, relay_mask, relay_state);
}

static SystemCanDevice s_device_lookup[NUM_EE_RELAY_IDS] = {
  [EE_RELAY_ID_BATTERY] = SYSTEM_CAN_DEVICE_BMS_CARRIER,
  [EE_RELAY_ID_MOTOR_CONTROLLER] = SYSTEM_CAN_DEVICE_BMS_CARRIER,
  [EE_RELAY_ID_SOLAR] = SYSTEM_CAN_DEVICE_SOLAR
};

SystemCanDevice *test_get_acking_device_lookup(void) {
  return s_device_lookup;
}

StatusCode relay_tx_relay_state(RelayTxStorage *storage, RetryTxRequest *request, EERelayId relay_id,
                                EERelayState state) {
  if (relay_id >= NUM_EE_RELAY_IDS || state >= NUM_EE_RELAY_STATES) {
    return STATUS_CODE_INVALID_ARGS;
  }
  storage->relay_id = relay_id;
  storage->relay_state = state;
  CanTxRetryWrapperRequest retry_wrapper_request = {
    .retry_request = *request,
    .ack_bitset = CAN_ACK_EXPECTED_DEVICES(s_device_lookup[relay_id]),
    .tx_callback = prv_tx_relay_state,
    .tx_callback_context = storage
  };
  can_tx_retry_send(&storage->can_retry_wrapper_storage, &retry_wrapper_request);
  return STATUS_CODE_OK;
}
