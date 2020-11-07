#include "mci_output_tx.h"
#include "can_transmit.h"
#include "log.h"

static void prv_tx_mci_output(CanAckRequest *ack_ptr, void *context) {
  MciOutputTxStorage *storage = (MciOutputTxStorage *)context;
  CAN_TRANSMIT_DRIVE_OUTPUT(ack_ptr, storage->drive_output);
}

StatusCode mci_output_init(MciOutputTxStorage *storage) {
  CanTxRetryWrapperSettings retry_settings = { .retries = NUM_MCI_OUTPUT_TX_RETRIES };
  status_ok_or_return(
      can_tx_retry_wrapper_init(&storage->can_retry_wrapper_storage, &retry_settings));
  return STATUS_CODE_OK;
}

static const char *s_ee_drive_output_names[] = {
  [EE_DRIVE_OUTPUT_OFF] = "off",
  [EE_DRIVE_OUTPUT_DRIVE] = "drive",
  [EE_DRIVE_OUTPUT_REVERSE] = "reverse",
};

StatusCode mci_output_tx_drive_output(MciOutputTxStorage *storage, RetryTxRequest *request,
                                      EEDriveOutput drive_output) {
  if (drive_output >= NUM_EE_DRIVE_OUTPUTS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  storage->drive_output = drive_output;
  CanTxRetryWrapperRequest retry_wrapper_request = { .retry_request = *request,
                                                     //  .ack_bitset = CAN_ACK_EXPECTED_DEVICES(
                                                     //      SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER),
                                                     .ack_bitset = CAN_ACK_EXPECTED_DEVICES(
                                                         SYSTEM_CAN_DEVICE_BABYDRIVER),
                                                     .tx_callback = prv_tx_mci_output,
                                                     .tx_callback_context = storage };
  can_tx_retry_send(&storage->can_retry_wrapper_storage, &retry_wrapper_request);
  LOG_DEBUG("outputting to mci %s\n", s_ee_drive_output_names[drive_output]);
  return STATUS_CODE_OK;
}
