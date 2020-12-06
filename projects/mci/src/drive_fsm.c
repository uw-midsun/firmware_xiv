#include "drive_fsm.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fsm.h"
#include "log.h"
#include "mci_events.h"
#include "motor_controller.h"
#include "precharge_control.h"
#include "status.h"

static MciDriveFsmEvent s_drive_output_fsm_map[] = {
  [EE_DRIVE_OUTPUT_OFF] = MCI_DRIVE_FSM_EVENT_OFF,
  [EE_DRIVE_OUTPUT_DRIVE] = MCI_DRIVE_FSM_EVENT_DRIVE,
  [EE_DRIVE_OUTPUT_REVERSE] = MCI_DRIVE_FSM_EVENT_REVERSE,
};

FSM_DECLARE_STATE(state_off);
FSM_DECLARE_STATE(state_drive);
FSM_DECLARE_STATE(state_reverse);
FSM_DECLARE_STATE(state_cruise);

static bool prv_guard_throttle(const Fsm *fsm, const Event *e, void *context) {
  return get_precharge_state() == MCI_PRECHARGE_CHARGED;
}

FSM_STATE_TRANSITION(state_off) {
  FSM_ADD_GUARDED_TRANSITION(MCI_DRIVE_FSM_EVENT_DRIVE, prv_guard_throttle, state_drive);
  FSM_ADD_GUARDED_TRANSITION(MCI_DRIVE_FSM_EVENT_REVERSE, prv_guard_throttle, state_reverse);
}

FSM_STATE_TRANSITION(state_drive) {
  FSM_ADD_TRANSITION(MCI_DRIVE_FSM_EVENT_OFF, state_off);
  FSM_ADD_TRANSITION(MCI_DRIVE_FSM_EVENT_REVERSE, state_reverse);
  FSM_ADD_TRANSITION(MCI_DRIVE_FSM_EVENT_TOGGLE_CRUISE, state_cruise);
}

FSM_STATE_TRANSITION(state_reverse) {
  FSM_ADD_TRANSITION(MCI_DRIVE_FSM_EVENT_OFF, state_off);
  FSM_ADD_TRANSITION(MCI_DRIVE_FSM_EVENT_DRIVE, state_drive);
}

FSM_STATE_TRANSITION(state_cruise) {
  FSM_ADD_TRANSITION(MCI_DRIVE_FSM_EVENT_TOGGLE_CRUISE, state_drive);
  FSM_ADD_TRANSITION(MCI_DRIVE_FSM_EVENT_REVERSE, state_reverse);
  FSM_ADD_TRANSITION(MCI_DRIVE_FSM_EVENT_OFF, state_off);
}

static Fsm s_drive_fsm;
static EEDriveOutput s_current_drive_state;
static bool s_is_cruise;

static void prv_state_off_output(Fsm *fsm, const Event *e, void *context) {
  s_current_drive_state = EE_DRIVE_OUTPUT_OFF;
  s_is_cruise = false;
}
static void prv_state_drive_output(Fsm *fsm, const Event *e, void *context) {
  s_current_drive_state = EE_DRIVE_OUTPUT_DRIVE;
  s_is_cruise = false;
}
static void prv_state_reverse_output(Fsm *fsm, const Event *e, void *context) {
  s_current_drive_state = EE_DRIVE_OUTPUT_REVERSE;
  s_is_cruise = false;
}

static void prv_state_cruise_output(Fsm *fsm, const Event *e, void *context) {
  s_current_drive_state = EE_DRIVE_OUTPUT_DRIVE;
  s_is_cruise = true;
}

static void prv_init_drive_fsm() {
  fsm_state_init(state_off, prv_state_off_output);
  fsm_state_init(state_drive, prv_state_drive_output);
  fsm_state_init(state_reverse, prv_state_reverse_output);
  fsm_state_init(state_cruise, prv_state_cruise_output);
  fsm_init(&s_drive_fsm, "drive_fsm", &state_off, NULL);
}

bool drive_fsm_process_event(const Event *e) {
  return fsm_process_event(&s_drive_fsm, e);
}

static StatusCode prv_drive_output_rx(const CanMessage *msg, void *context,
                                      CanAckStatus *ack_reply) {
  (void)context;
  uint16_t drive_output = 0;
  bool expect_transition = true;
  CAN_UNPACK_DRIVE_OUTPUT(msg, &drive_output);
  Event e = { 0 };
  e.id = s_drive_output_fsm_map[drive_output];
  expect_transition = drive_output != s_current_drive_state;
  LOG_DEBUG("curent state: %i (target state: %i)\n", s_current_drive_state, drive_output);
  LOG_DEBUG("drive_fsm_state: %i\n", e.id);
  bool transitioned = drive_fsm_process_event(&e);
  LOG_DEBUG("post transition: %i\n", s_current_drive_state);
  if (expect_transition != transitioned) {
    *ack_reply = CAN_ACK_STATUS_INVALID;
    return STATUS_CODE_INTERNAL_ERROR;
  }
  *ack_reply = CAN_ACK_STATUS_OK;
  return STATUS_CODE_OK;
}

EEDriveOutput drive_fsm_get_drive_state() {
  return s_current_drive_state;
}

bool drive_fsm_is_cruise() {
  return s_is_cruise;
}

bool drive_fsm_toggle_cruise() {
  Event e = { .id = MCI_DRIVE_FSM_EVENT_TOGGLE_CRUISE };
  return drive_fsm_process_event(&e);
}

StatusCode drive_fsm_init() {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT, prv_drive_output_rx, NULL);

  prv_init_drive_fsm();
  return STATUS_CODE_OK;
}
