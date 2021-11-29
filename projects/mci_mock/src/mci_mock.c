#include "mci_mock.h"

#include "can.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fsm.h"
#include "mci_events.h"

static MciDriveFsmEvent s_drive_output_fsm_map[] = {
  [EE_DRIVE_OUTPUT_OFF] = MCI_DRIVE_FSM_EVENT_OFF,
  [EE_DRIVE_OUTPUT_DRIVE] = MCI_DRIVE_FSM_EVENT_DRIVE,
  [EE_DRIVE_OUTPUT_REVERSE] = MCI_DRIVE_FSM_EVENT_REVERSE,
};

FSM_DECLARE_STATE(state_off);
FSM_DECLARE_STATE(state_drive);
FSM_DECLARE_STATE(state_reverse);

FSM_STATE_TRANSITION(state_off) {
  FSM_ADD_TRANSITION(MCI_DRIVE_FSM_EVENT_DRIVE, state_drive);
  FSM_ADD_TRANSITION(MCI_DRIVE_FSM_EVENT_REVERSE, state_reverse);
}

FSM_STATE_TRANSITION(state_drive) {
  FSM_ADD_TRANSITION(MCI_DRIVE_FSM_EVENT_OFF, state_off);
  FSM_ADD_TRANSITION(MCI_DRIVE_FSM_EVENT_REVERSE, state_reverse);
}

FSM_STATE_TRANSITION(state_reverse) {
  FSM_ADD_TRANSITION(MCI_DRIVE_FSM_EVENT_OFF, state_off);
  FSM_ADD_TRANSITION(MCI_DRIVE_FSM_EVENT_DRIVE, state_drive);
}

static Fsm s_drive_fsm;
static EEDriveOutput s_current_drive_state;

static void prv_state_off_output(Fsm *fsm, const Event *e, void *context) {
  s_current_drive_state = EE_DRIVE_OUTPUT_OFF;
}

static void prv_state_drive_output(Fsm *fsm, const Event *e, void *context) {
  s_current_drive_state = EE_DRIVE_OUTPUT_DRIVE;
}

static void prv_state_reverse_output(Fsm *fsm, const Event *e, void *context) {
  s_current_drive_state = EE_DRIVE_OUTPUT_REVERSE;
}

static StatusCode prv_init_drive_fsm() {
  fsm_state_init(state_off, prv_state_off_output);
  fsm_state_init(state_drive, prv_state_drive_output);
  fsm_state_init(state_reverse, prv_state_reverse_output);
  fsm_init(&s_drive_fsm, "drive_fsm", &state_off, NULL);
  return STATUS_CODE_OK;
}

bool drive_fsm_process_event(const Event *e) {
  return fsm_process_event(&s_drive_fsm, e);
}

static StatusCode prv_drive_output_rx(const CanMessage *msg, void *context,
                                      CanAckStatus *ack_reply) {
  uint8_t drive_output = 0;
  for (uint8_t i = 0; i < msg->dlc; i++) {
    drive_output += ((msg->data >> (i * 8)) & 0xFF) << (i * 8);
  }
  bool expect_transition = drive_output != s_current_drive_state;

  Event e = { 0 };
  e.id = s_drive_output_fsm_map[drive_output];

  bool transitioned = drive_fsm_process_event(&e);
  if (transitioned != expect_transition) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode drive_fsm_init() {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT, prv_drive_output_rx, NULL);

  prv_init_drive_fsm();
  return STATUS_CODE_OK;
}

EEDriveOutput drive_fsm_get_drive_state() {
  return s_current_drive_state;
}
