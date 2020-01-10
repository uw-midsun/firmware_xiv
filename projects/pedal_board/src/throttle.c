#include "throttle.h" 

#include <string.h> 
#include <stdio.h> 
#include <stdbool.h> 
#include <limits.h>

#include "event_queue.h"
#include "log.h"
#include "pedal_events.h" 
#include "soft_timer.h"

static void prv_timer_callback(SoftTimerId timer_id, void *context){
    ThrottleStorage *storage = context; 
    int16_t reading = INT16_MAX; 
    
    StatusCode read_status = ads1015_read_raw(&(storage->ads_storage), storage->ads_channel, &reading); 
    
    storage->position_raw = reading; 

    uint16_t u_reading = (uint16_t)reading; // Cast reading from int to uint, as ads1015_read_raw assigns a signed int 
                                            // while event_raise only accepts an unsigned int for the data field 

    //TO-DO(SOFT-18): map raw readings to a value that represents throttle position before raising  
    event_raise(PEDAL_EVENT_THROTTLE_READING, u_reading); // 

    soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_timer_callback, context, drive_fsm_soft_timer_id); 
}

StatusCode throttle_init(ThrottleStorage *storage){
    
    storage->state = THROTTLE_STATE_ENABLE; 

    StatusCode ret = soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_timer_callback, storage, drive_fsm_soft_timer_id); 

    return ret; 
}

StatusCode throttle_enable(ThrottleStorage *storage){
    storage->state = THROTTLE_STATE_ENABLE; 

    StatusCode ret = soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_timer_callback, storage, drive_fsm_soft_timer_id); 

    return ret; 
}

StatusCode throttle_disable(ThrottleStorage *storage){
    storage->state = THROTTLE_STATE_DISABLE; 

    bool cancel_timer = soft_timer_cancel(drive_fsm_soft_timer_id); 

    if (cancel_timer) return STATUS_CODE_OK; 
    else return STATUS_CODE_UNINITIALIZED; 
}