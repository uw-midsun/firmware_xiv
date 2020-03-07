#pragma once

#include "event_queue.h"
#include "status.h"

StatusCode begin_charge_fsm_init();

bool begin_fsm_process_event(const Event *e);
