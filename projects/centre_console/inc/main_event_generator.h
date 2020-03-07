#pragma once

#include "status.h"
#include "stdbool.h"
#include "event_queue.h"

StatusCode main_event_generator_init(void);

bool main_event_generator_process_event(const Event *event);

