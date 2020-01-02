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
    
    if(storage->state == THROTTLE_STATE_ENABLE){
        StatusCode read_status = ads1015_read_raw(&(storage->ads_storage), storage->ads_channel, &reading); 
       
        storage->position_raw = reading; 

        uint16_t u_reading = (uint16_t)reading; // Cast reading from int to uint, as ads1015_read_raw assigns a signed int 
                                                // while event_raise only accepts an unsigned int for the data field  
        event_raise(PEDAL_EVENT_THROTTLE_READING, u_reading); // Will map readings later 
    }
    soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_timer_callback, context, NULL); 
}

// Private function to map reading data from ads1015 to position of pedal 
/*static void prv_map_raw_reading(ThrottleStorage *storage, int16_t reading){
    
}*/

StatusCode throttle_init(ThrottleStorage *storage){
    
    storage->state = THROTTLE_STATE_ENABLE; 

    StatusCode ret = soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_timer_callback, storage, NULL); 

    return ret; 
}

StatusCode throttle_enable(ThrottleStorage *storage){
    storage->state = THROTTLE_STATE_ENABLE; 

    return STATUS_CODE_OK; 
}

StatusCode throttle_disable(ThrottleStorage *storage){
    storage->state = THROTTLE_STATE_DISABLE; 

    return STATUS_CODE_OK; 
}

/*StatusCode throttle_get_position(ThrottleStorage *throttle_storage, ThrottleState throttle_state, int32_t *position){

}*/ 