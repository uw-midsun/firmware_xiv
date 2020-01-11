#include "ads1015.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "events.h"

Fsm *brake_fsm;

FSM_DECLARE_STATE(pressed);
FSM_DECLARE_STATE(released);

FSM_STATE_TRANSITION(pressed) {
  FSM_ADD_TRANSITION(BRAKE_RELEASED, released);
};

FSM_STATE_TRANSITION(released) {
  FSM_ADD_TRANSITION(BRAKE_PRESSED, pressed);
}

static void prv_callback_channel1(Ads1015Channel channel, void *context) {
  Ads1015Storage *storage = context;
  int16_t position = 0;
  ads1015_read_raw(storage, channel, &position);

  // math to convert readings to angles
  // position =

  if (position > (3.14 / 4)) {
    event_raise(BRAKE_PRESSED, 1);
  } else {
    event_raise(BRAKE_RELEASED, 0);
  }
}

static void prv_pressed_output(Fsm *fsm, const Event *e, void *context) {
  // raises a can event to main, which sends can message
  event_raise(CAN_BRAKE_PRESSED, 1);
}

static void prv_released_output(Fsm *fsm, const Event *e, void *context) {
  // raises a can event to main, which sends can message
  event_raise(CAN_BRAKE_RELEASED, 0);
}

bool brake_fsm_process_event(Event *e) {
  // transition the state
  return fsm_process_event(brake_fsm, e);
}

//main should have a brake fsm, and ads1015storage
StatusCode brake_fsm_init(Fsm *brake, Ads1015Storage *storage) {
  fsm_state_init(pressed, prv_pressed_output);
  fsm_state_init(released, prv_released_output);
  brake_fsm = brake;

  fsm_init(brake, "Brake_FSM", &released, brake);

  return ads1015_configure_channel(storage, storage->current_channel, true, prv_callback_channel1, storage);
}
