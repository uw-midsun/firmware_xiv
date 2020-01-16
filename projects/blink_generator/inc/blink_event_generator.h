#pragma once

#include "event_queue.h"
#include "lights_events.h"
#include "soft_timer.h"

// States for the blinker
typedef enum {
  LIGHTS_BLINK_STATE_OFF = 0,
  LIGHTS_BLINK_STATE_ON,
  NUM_LIGHTS BLINK_STATES
} LightsBlinkerState;


typedef uint32_t LightsBlinkerDuration;

// Blinker storage structure
typedef struct LightsBlinker {
  
}
