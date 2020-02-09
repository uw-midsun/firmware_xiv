#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fsm.h"
#include "motor_controller.h"
#include "precharge_control.h"
#include "status.h"

#include "drive_fsm.h"

static DriveFsmState s_drive_output_fsm_map[] = {
    [EE_DRIVE_OUTPUT_OFF] = DRIVE_FSM_STATE_NEUTRAL,
    [EE_DRIVE_OUTPUT_DRIVE] = DRIVE_FSM_STATE_DRIVE,
    [EE_DRIVE_OUTPUT_REVERSE] = DRIVE_FSM_STATE_REVERSE
};

FSM_DECLARE_STATE(state_neutral);
FSM_DECLARE_STATE(state_drive);
FSM_DECLARE_STATE(state_reverse);

static bool prv_guard_throttle(const Fsm *fsm, const Event *e, void *context) {
  MotorControllerStorage *storage = context;
  if (storage->precharge_storage.state != MCI_PRECHARGE_CHARGED) {
    return false;
  }
  return true;
}

FSM_STATE_TRANSITION(state_neutral) {
  FSM_ADD_GUARDED_TRANSITION(DRIVE_FSM_STATE_DRIVE, prv_guard_throttle, state_drive);
  FSM_ADD_GUARDED_TRANSITION(DRIVE_FSM_STATE_REVERSE, prv_guard_throttle, state_reverse);
}

FSM_STATE_TRANSITION(state_drive) {
  FSM_ADD_TRANSITION(DRIVE_FSM_STATE_NEUTRAL, state_neutral);
  FSM_ADD_TRANSITION(DRIVE_FSM_STATE_REVERSE, state_reverse);
}

FSM_STATE_TRANSITION(state_reverse) {
  FSM_ADD_TRANSITION(DRIVE_FSM_STATE_NEUTRAL, state_neutral);
  FSM_ADD_TRANSITION(DRIVE_FSM_STATE_DRIVE, state_drive);
}

static void prv_state_neutral_output(Fsm *fsm, const Event *e, void *context) {
  MotorControllerStorage *storage = context;
  storage->drive_state = EE_DRIVE_OUTPUT_OFF;
}
static void prv_state_drive_output(Fsm *fsm, const Event *e, void *context) {
  MotorControllerStorage *storage = context;
  storage->drive_state = EE_DRIVE_OUTPUT_DRIVE;
}
static void prv_state_reverse_output(Fsm *fsm, const Event *e, void *context) {
  MotorControllerStorage *storage = context;
  storage->drive_state = EE_DRIVE_OUTPUT_REVERSE;
}

static Fsm s_drive_fsm;
static void prv_init_drive_fsm(void *context) {
  fsm_state_init(state_neutral, prv_state_neutral_output);
  fsm_state_init(state_drive, prv_state_drive_output);
  fsm_state_init(state_reverse, prv_state_reverse_output);
  fsm_init(&s_drive_fsm, "drive_fsm", &state_neutral, context);
}

bool drive_fsm_process_event(const Event *e) {
  return fsm_process_event(&s_drive_fsm, e);
}

StatusCode drive_output_rx(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  uint16_t drive_output = 0;
  CAN_UNPACK_DRIVE_OUTPUT(msg, &drive_output);
  Event e = { 0 };
  e.id = s_drive_output_fsm_map[drive_output];

  bool transitioned = drive_fsm_process_event(&e);
  bool ret = STATUS_CODE_OK;
  if (transitioned != true) {
    *ack_reply = CAN_ACK_STATUS_INVALID;
    ret = STATUS_CODE_INTERNAL_ERROR;
  }
  *ack_reply = CAN_ACK_STATUS_OK;
  return ret;
}

// StatusCode fault_rx(const CanMessage *msg, void *context, CanAckStatus *ack_status) {
//   Event e = {.id = DRIVE_FSM_STATE_NEUTRAL };
//   drive_fsm_process_event(&e);
//   *ack_status = CAN_ACK_STATUS_OK;
//   return STATUS_CODE_OK;
// }

StatusCode drive_fsm_init(void *context) {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT, drive_output_rx, context);
  // TODO(SOFT-70): add rx handlers for all potential faults using fault_rx()
  prv_init_drive_fsm(context);
  return STATUS_CODE_OK;
}
