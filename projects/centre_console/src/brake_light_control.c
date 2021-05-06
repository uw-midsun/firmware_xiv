#include "brake_light_control.h"

#include "can.h"
#include "can_transmit.h"
#include "centre_console_events.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "pedal_monitor.h"
#include "pedal_rx.h"
#include "soft_timer.h"
#include "status.h"

// Called when brake light should change
// Will transmit the appropriate CanMessage to turn brake lights on/off
static void prv_can_transmit_brake_light_change(PedalState current_state) {
  if (current_state == PEDAL_STATE_RELEASED) {
    CAN_TRANSMIT_LIGHTS(SYSTEM_CAN_MESSAGE_LIGHTS, EE_LIGHT_STATE_OFF);

  } else if (current_state == PEDAL_STATE_PRESSED) {
    CAN_TRANSMIT_LIGHTS(SYSTEM_CAN_MESSAGE_LIGHTS, EE_LIGHT_STATE_ON);
  }
}

// Processes PEDAL_MONITOR_STATE_CHANGE events and will transmit
// the appropriate CanMessage to turn brake lights on/off
bool brake_light_control_process_event(Event *e) {
  if (e != NULL && e->id == PEDAL_MONITOR_STATE_CHANGE) {
    prv_can_transmit_brake_light_change(e->data);
    return true;
  }
  return false;
}
