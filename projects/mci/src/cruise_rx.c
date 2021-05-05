#include "cruise_rx.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "critical_section.h"
#include "drive_fsm.h"
#include "exported_enums.h"
#include "fsm.h"
#include "log.h"
#include "status.h"

float s_target_velocity_ms;

void cruise_rx_update_velocity(float current_velocity_ms) {
  if (!drive_fsm_is_cruise()) {
    bool disabled = critical_section_start();
    s_target_velocity_ms = current_velocity_ms;
    critical_section_end(disabled);
  }
}

float cruise_rx_get_target_velocity(void) {
  return s_target_velocity_ms;
}

static StatusCode prv_cruise_command_rx(const CanMessage *msg, void *context,
                                        CanAckStatus *ack_reply) {
  (void)context;
  uint8_t cruise_command = 0;
  CAN_UNPACK_CRUISE_CONTROL_COMMAND(msg, &cruise_command);

  switch (cruise_command) {
    case EE_CRUISE_CONTROL_COMMAND_TOGGLE:
      if (s_target_velocity_ms >= MCI_MIN_CRUISE_VELOCITY_MS &&
          s_target_velocity_ms <= MCI_MAX_CRUISE_VELOCITY_MS)
        drive_fsm_toggle_cruise();
      return STATUS_CODE_OK;
    case EE_CRUISE_CONTROL_COMMAND_INCREASE:
      if (drive_fsm_is_cruise())
        s_target_velocity_ms =
            MIN(MCI_MAX_CRUISE_VELOCITY_MS, s_target_velocity_ms + MCI_CRUISE_CHANGE_AMOUNT_MS);
      return STATUS_CODE_OK;
    case EE_CRUISE_CONTROL_COMMAND_DECREASE:
      if (drive_fsm_is_cruise())
        s_target_velocity_ms =
            MAX(MCI_MIN_CRUISE_VELOCITY_MS, s_target_velocity_ms - MCI_CRUISE_CHANGE_AMOUNT_MS);
      return STATUS_CODE_OK;
    default:
      return STATUS_CODE_UNKNOWN;
  }
}

StatusCode cruise_rx_init() {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_CRUISE_CONTROL_COMMAND, prv_cruise_command_rx, NULL);
  return STATUS_CODE_OK;
}
