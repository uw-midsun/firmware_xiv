#pragma once

// requires gpio and the event queue to be initialized

#include "event_queue.h"
#include "status.h"

#define CONTROL_PILOT_GPIO_ADDR \
  { GPIO_PORT_A, 6 }

void control_pilot_process_event(Event *e);

StatusCode control_pilot_init();
