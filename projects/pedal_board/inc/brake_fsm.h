#include "fsm.h"
#include "ads1015.h"

//initialize the brake fsm
StatusCode brake_fsm_init(Fsm *brake, Ads1015Storage *storage);

//process the fsm event
bool brake_fsm_process_event(Event *e);