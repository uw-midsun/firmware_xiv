#pragma once

#include "event_queue.h"
#include "status.h"

#define numStates 3

typedef enum {
    EVENT_DRIVE_PRESSED = 0,
    EVENT_NEUTRAL_PRESSED,
    EVENT_REVERSE_PRESSED,
} EVENT_BUTTON_PRESSED;

typedef enum {
    DRIVE_BUTTON = 0,
    NEUTRAL_BUTTON,
    REVERSE_BUTTON,
} DRIVE_STATE_BUTTON;

//used in main loop to raise and process events based on button inputs
bool drive_buttons_process_event(const Event *e);

StatusCode drive_buttons_init();

