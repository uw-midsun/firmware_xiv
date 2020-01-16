#include <stddef.h>

#include "event_queue.h"
#include "soft_timer.h"
#include "status.h"

#include "lights_events.h"
#include "blink_event_generator.h"


static void prv_timer_callback(){
  // if the blinker is active, call this function again
  // and raise the event opposite to the current state
  if
}

// Initializes the blink_event_generator
// Needs to raise an event and calls blink_event_generator_deactivate
void blink_event_generator_init(){

}

// Activates the blink state
// Calls prv_timer_callback
void blink_event_generator_activate(){
  // Needs storage and event?

}

// Deactivates the blink state
// Does not call prv_timer_callback
void blink_event_generator_deactivate(){
  // Needs storage?

}

// returns 0 if not active or 1 if active
// Unsure if needed
static bool prv_lights_blinker_is_active(){

}
