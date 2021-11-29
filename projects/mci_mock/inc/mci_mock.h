#pragma once

#include "event_queue.h"
#include "exported_enums.h"
#include "status.h"

bool drive_fsm_process_event(const Event *e);

StatusCode drive_fsm_init();

EEDriveOutput drive_fsm_get_drive_state();
