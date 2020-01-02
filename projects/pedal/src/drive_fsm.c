#include "drive_fsm.h"

#include "pedal_events.h"
#include "throttle.h"


FSM_DECLARE_STATE(state_enable);
FSM_DECLARE_STATE(state_disable); 

FSM_STATE_TRANSITION(state_enable){
    FSM_ADD_TRANSITION(PEDAL_EVENT_THROTTLE_DISABLE, state_disable);
}

FSM_STATE_TRANSITION(state_disable){
    FSM_ADD_TRANSITION(PEDAL_EVENT_THROTTLE_ENABLE, state_enable); 
} 

static void prv_state_enable_output(Fsm *fsm, const Event *e, void *context){
    ThrottleStorage *storage = context; 
    throttle_enable(storage); 
   
} 

static void prv_state_disable_output(Fsm *fsm, const Event *e, void *context){
    ThrottleStorage *storage = context; 
    throttle_disable(storage); 
 
} 

StatusCode drive_fsm_init(Fsm *fsm, ThrottleStorage *storage){
    fsm_state_init(state_enable, prv_state_enable_output); 
    fsm_state_init(state_disable, prv_state_disable_output);
    
    fsm_init(fsm, "Drive FSM", &state_enable, storage); 

    return STATUS_CODE_OK; 
} 

