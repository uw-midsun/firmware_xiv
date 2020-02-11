#include "relay_tx.h"

StatusCode relay_tx_init(RelayTxStorage *storage) {
  return STATUS_CODE_OK;
}

StatusCode relay_tx_relay_state(RelayTxStorage *storage, RetryTxRequest *request,
                                EERelayState state) {
  return STATUS_CODE_OK;
}
