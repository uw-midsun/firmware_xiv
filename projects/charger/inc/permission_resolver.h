#pragma once
#include "status.h"
#include "event_queue.h"

bool permission_resolver_process_event(const Event *e);
StatusCode permission_resolver_init();
