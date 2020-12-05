#pragma once

#include "event_queue.h"
#include "exported_enums.h"
#include "status.h"

StatusCode drive_fsm_init(void);

bool drive_fsm_process_event(const Event *e);

EEDriveOutput drive_fsm_get_drive_state(void);

bool drive_fsm_is_cruise(void);

bool drive_fsm_toggle_cruise();
