#include "begin_charge_fsm.h"
#include "charger_events.h"

#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "status.h"

static GpioAddress s_control_pilot_pin = { .port = GPIO_PORT_A, .pin = 2 };

FSM_DECLARE_STATE(state_idle);
FSM_DECLARE_STATE(state_pending);

FSM_STATE_TRANSITION(state_idle) {
  FSM_ADD_TRANSITION(BEGIN_CHARGE_FSM_EVENT_PENDING_STATE, state_pending);
}

FSM_STATE_TRANSITION(state_pending) {
  FSM_ADD_TRANSITION(CHARGER_PWM_EVENT_VALUE_AVAILABLE, state_idle);
}

static Fsm s_begin_charge_fsm;

static void prv_state_pending_output(Fsm *fsm, const Event *e, void *context) {
  // raises reading request
  event_raise(CHARGER_PWM_EVENT_REQUEST_READING, 0);
}

static void prv_state_idle_output(Fsm *fsm, const Event *e, void *context) {
  // get max current from event
  uint16_t max_current = e->data;
  // set control pilot state
  gpio_set_state(&s_control_pilot_pin, GPIO_STATE_HIGH);
  // TODO(SOFT-130): activate charger controller
}

StatusCode begin_charge_fsm_init() {
  fsm_state_init(state_idle, prv_state_idle_output);
  fsm_state_init(state_pending, prv_state_pending_output);
  fsm_init(&s_begin_charge_fsm, "begin_charge_fsm", &state_idle, NULL);
  return STATUS_CODE_OK;
}

bool begin_fsm_process_event(const Event *e) {
  return fsm_process_event(&s_begin_charge_fsm, e);
}
