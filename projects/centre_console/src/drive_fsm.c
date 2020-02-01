#include "drive_fsm.h"
#include "centre_console_events.h"
#include "fsm.h"

// neutral -> drive/reverse
// 1. close relays
// 2. BOOM we're in drive
// 3. broadcast (WE'RE IN DRIVE)
// 4. Powertrain heartbeat
// parking -> drive/reverse
// 1. exit parking 
// 2. same as neutral->drive
// drive -> neutral
// 1. open relays
// 2. BOOM we're neutral
// 3. broadcast (WE'RE IN NEUTRAL)
// 4. stop powertrain heartbeat
// drive -> parking
// 1. same as drive->neutral
// 2. apply handbrake
// 3. broadcast, raise event, etc.

FSM_DECLARE_STATE(state_drive);
FSM_DECLARE_STATE(state_reverse);
FSM_DECLARE_STATE(state_parking);
FSM_DECLARE_STATE(state_neutral);
FSM_DECLARE_STATE(state_set_relays);
FSM_DECLARE_STATE(state_set_ebrake);

FSM_STATE_TRANSITION(state_neutral) {
  FSM_ADD_TRANSITION(DRIVE_EVENT_PARKING, state_set_ebrake);
  FSM_ADD_TRANSITION(DRIVE_EVENT_REVERSE, state_set_relays);
  FSM_ADD_TRANSITION(DRIVE_EVENT_DRIVE, state_set_relays);
}

FSM_STATE_TRANSITION(state_drive) {
  FSM_ADD_TRANSITION(DRIVE_EVENT_PARKING, state_set_relays);
  FSM_ADD_TRANSITION(DRIVE_EVENT_REVERSE, state_reverse);
  FSM_ADD_TRANSITION(DRIVE_EVENT_NEUTRAL, state_neutral);
}

FSM_STATE_TRANSITION(state_parking) {
  FSM_ADD_TRANSITION(DRIVE_EVENT_REVERSE, state_set_ebrake);
  FSM_ADD_TRANSITION(DRIVE_EVENT_NEUTRAL, state_set_ebrake);
  FSM_ADD_TRANSITION(DRIVE_EVENT_DRIVE, state_set_ebrake);
}

FSM_STATE_TRANSITION(state_reverse) {
  FSM_ADD_TRANSITION(DRIVE_EVENT_PARKING, state_set_relays);
  FSM_ADD_TRANSITION(DRIVE_EVENT_NEUTRAL, state_set_relays);
  FSM_ADD_TRANSITION(DRIVE_EVENT_DRIVE, state_drive);
}

static bool prv_destination_guard(const Fsm *fsm, const Event *e, void *context) {
  DriveFsmStorage *storage = (DriveFsmStorage *)context;
  return e->data == storage->destination;
}

FSM_STATE_TRANSITION(state_set_relays) {
  FSM_ADD_GUARDED_TRANSITION(DRIVE_EVENT_MCI_RELAYS_CLOSED, prv_destination_guard, state_drive);
  FSM_ADD_GUARDED_TRANSITION(DRIVE_EVENT_MCI_RELAYS_CLOSED, prv_destination_guard, state_reverse);
  FSM_ADD_GUARDED_TRANSITION(DRIVE_EVENT_MCI_RELAYS_OPENED, prv_destination_guard, state_neutral);
  FSM_ADD_GUARDED_TRANSITION(DRIVE_EVENT_MCI_RELAYS_OPENED, prv_destination_guard, state_set_ebrake);
}


StatusCode power_main_sequence_init(PowerMainSequenceFsmStorage *storage) {
  prv_init_ack_count();
  prv_init_ack_lookup();
  power_main_precharge_monitor_init(&storage->precharge_monitor_storage);
  fsm_state_init(state_none, prv_state_none);
  fsm_state_init(state_fault, prv_state_fault);
  storage->current_sequence = EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS;
  fsm_init(&storage->sequence_fsm, "power_main_sequence_fsm", &state_none, storage);
  return STATUS_CODE_OK;
}





