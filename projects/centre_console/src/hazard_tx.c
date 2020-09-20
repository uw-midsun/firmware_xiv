#include "hazard_tx.h"

#include "can_transmit.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "exported_enums.h"

// This could be an FSM, but that would be overkill for a single bool.

static bool s_hazard_on = false;

static void toggle_hazard(void) {
  s_hazard_on = !s_hazard_on;
  CAN_TRANSMIT_HAZARD(s_hazard_on ? EE_LIGHT_STATE_ON : EE_LIGHT_STATE_OFF);
}

void hazard_tx_init(void) {
  s_hazard_on = false;
}

bool hazard_tx_process_event(Event *e) {
  if (e != NULL && e->id == CENTRE_CONSOLE_BUTTON_PRESS_EVENT_HAZARD) {
    toggle_hazard();
    return true;
  }
  return false;
}
