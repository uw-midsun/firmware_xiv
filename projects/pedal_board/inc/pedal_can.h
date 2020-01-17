#pragma once
#include "can.h"
#include "event_queue.h"
#include "log.h"

StatusCode pedal_can_init(CanStorage *can_storage, CanSettings *can_settings);

bool pedal_can_process_event(Event *e);
