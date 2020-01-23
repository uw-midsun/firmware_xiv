#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "fsm.h"
#include "event_queue.h"


typedef enum {
    EVENT_STALK_ON,
    EVENT_STALK_OFF,
    EVENT_HAZARD_PRESSED,
    EVENT_HAZARD_RELEASED,
    NUM_EVENTS,
} SignalEvents

FSM_DECLARE_STATE(state_none);
FSM_DECLARE_STATE(state_signal_left);
FSM_DECLARE_STATE(state_signal_left_hazard);
FSM_DECLARE_STATE(state_signal_right);
FSM_DECLARE_STATE(state_signal_right_hazard);
FSM_DECLARE_STATE(state_hazard);


FSM_STATE_TRANSITION(state_none) {
    FSM_ADD_TRANSITION(EVENT_STALK_ON, state_signal_left);
    FSM_ADD_TRANSITION(EVENT_STALK_OFF, state_signal_right);
    FSM_ADD_TRANSITION(EVENT_HAZARD_PRESSED, state_hazard);
    
}

FSM_STATE_TRANSITION(state_signal_left) {
    FSM_ADD_TRANSITION(EVENT_HAZARD_PRESSED, state_signal_left_hazard);
    FSM_ADD_TRANSITION(EVENT_STALK_OFF, state_none);
}

FSM_STATE_TRANSITION(state_signal_left_hazard) {
    FSM_ADD_TRANSITION(EVENT_HAZARD_RELEASED, state_signal_left);
    FSM_ADD_TRANSITION(EVENT_STALK_OFF, state_hazard);
}

FSM_STATE_TRANSITION(state_signal_right) {
    FSM_ADD_TRANSITION(EVENT_HAZARD_PRESSED, state_signal_right_hazard);
    FSM_ADD_TRANSITION(EVENT_STALK_ON, state_none);
}

FSM_STATE_TRANSITION(state_signal_right_hazard) {
    FSM_ADD_TRANSITION(EVENT_HAZARD_RELEASED, state_signal_right);
    FSM_ADD_TRANSITION(EVENT_STALK_OFF, state_hazard);
}

FSM_STATE_TRANSITION(state_hazard) {
    FSM_ADD_TRANSITION(EVENT_HAZARD_RELEASED, state_none);
    FSM_ADD_TRANSITION(EVENT_STALK_OFF, state_hazard);
}


static void state_none_output() {
    //do something 
}

static void state_signal_left_output() {
    //do something 
}

static void state_signal_left_hazard_output() {
    //do something 
}

static void state_signal_right_output() {
    //do something 
}

static void state_signal_right_hazard_output() {
    //do something 
}

static void state_hazard_output() {
    //do something 
}


static Fsm s_fsm; //where do these come from?
static void prv_init_fsm(void) {
    fms_state_init(state_none, state_none_output);
    fms_state_init(state_signal_left, state_signal_left_output);
    fms_state_init(state_signal_left_hazard, state_signal_left_hazard_output);
    fms_state_init(state_signal_right, state_signal_right_output);
    fms_state_init(state_signal_right_hazard, state_signal_right_hazard_output);
    fms_state_init(state_hazard, state_hazard_output);
}



