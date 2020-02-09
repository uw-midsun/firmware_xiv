#include "ebrake_tx.h"

StatusCode ebrake_tx_init(EbrakeTxStorage *storage) {
  return STATUS_CODE_OK;
}

StatusCode ebrake_tx_brake_state(EbrakeTxStorage *storage, RetryTxRequest *request,
                                 EEEbrakeState state) {
  return STATUS_CODE_OK;
}
