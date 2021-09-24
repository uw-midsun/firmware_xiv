#include "hazard_tx.h"

#include "can_transmit.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"

static void toggle_hazard(bool active) {
  // Raising CAN event last for ease of testing, so that the loopback event sequence is
  // hazard event -> TX event -> RX event instead of TX event -> hazard event -> RX event
  event_raise_no_data(active ? HAZARD_EVENT_ON : HAZARD_EVENT_OFF);
  CAN_TRANSMIT_HAZARD(active ? EE_LIGHT_STATE_ON : EE_LIGHT_STATE_OFF);
}

bool hazard_tx_process_event(Event *e) {
  if (e != NULL && e->id == CENTRE_CONSOLE_BUTTON_PRESS_EVENT_HAZARD) {
    GpioState button_state = e->data;
    toggle_hazard(button_state == HAZARD_BUTTON_ACTIVE_STATE);
    return true;
  }
  return false;
}
