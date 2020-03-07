#pragma once

#include "event_queue.h"
#include "status.h"

#define PWM_TOLERANCE (2)
#define PWM_READ_PERIOD_US 1000

void control_pilot_monitor_process_event(Event *e);

StatusCode control_pilot_monitor_init();
