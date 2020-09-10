#include "bps_heartbeat.h"

#include "bms.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "exported_enums.h"
#include "fault_bps.h"
#include "soft_timer.h"

static uint32_t s_hb_freq_ms = 0;

static StatusCode prv_handle_ack(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                 uint16_t num_remaining, void *context) {
  BpsStorage *storage = context;
  if (status != CAN_ACK_STATUS_OK) {
    storage->ack_fail_count++;

    if (storage->ack_fail_count >= BPS_ACK_MAX_FAILS) {
      fault_bps(EE_BPS_STATE_FAULT_ACK_TIMEOUT, false);
    }
  } else if (num_remaining == 0) {
    storage->ack_fail_count = 0;
    fault_bps(EE_BPS_STATE_FAULT_ACK_TIMEOUT, true);
  }

  return STATUS_CODE_OK;
}

static void prv_periodic_heartbeat(SoftTimerId timer_id, void *context) {
  BpsStorage *storage = context;

  CanAckRequest ack_req = {
    .callback = prv_handle_ack, .context = storage, .expected_bitset = storage->ack_devices
  };

  CAN_TRANSMIT_BPS_HEARTBEAT(&ack_req, storage->fault_bitset);

  soft_timer_start_millis(s_hb_freq_ms, prv_periodic_heartbeat, storage, 0);
}

StatusCode bps_heartbeat_init(BpsStorage *storage, uint32_t hb_freq_ms) {
  // force first one
  s_hb_freq_ms = hb_freq_ms;
  prv_periodic_heartbeat(0, storage);

  return STATUS_CODE_OK;
}
