#include "smoke_charger_comms.h"

#include "can.h"
#include "can_transmit.h"
#include "can_msg_defs.h"
#include "charger_controller.h"
#include "event_queue.h"
#include "gpio.h"
#include "log.h"
#include "soft_timer.h"

void smoke_charger_controll_perform(void){
    gpio_init();
    soft_timer_init();
    event_queue_init();
    can_init();
    charger_controller_init();
    
}
