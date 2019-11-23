#include "fsm.h"
#include "event_queue.h"


static Fsm drive_state_buttons_fsm;

FSM_DECLARE_STATE(drive_pressed);
FSM_DECLARE_STATE(neutral_pressed);
FSM_DECLARE_STATE(reverse_pressed);

FSM_STATE_TRANSITION(drive_pressed){
    FSM_ADD_TRANSITION(EVENT_NEUTRAL_PRESSED, neutral_pressed);
}

FSM_STATE_TRANSITION(neutral_pressed){
    FSM_ADD_TRANSITION(EVENT_DRIVE_PRESSED, drive_pressed);
    FSM_ADD_TRANSITION(EVENT_REVERSE_PRESSED, reverse_pressed);
}

FSM_STATE_TRANSITION(reverse_pressed){
    FSM_ADD_TRANSITION(EVENT_NEUTRAL_PRESSED, neutral_pressed);
}
