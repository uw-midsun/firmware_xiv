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

static PedalState s_prev_pedal_state = PEDAL_STATE_RELEASED;

// Called when brake light should change
// Will transmit the appropriate CanMessage to turn brake lights on/off
static void prv_can_transmit_brake_light_change(PedalState current_state) {
  if (current_state == PEDAL_STATE_RELEASED) {
    CAN_TRANSMIT_LIGHTS(SYSTEM_CAN_MESSAGE_LIGHTS, EE_LIGHT_STATE_OFF);

  } else if (current_state == PEDAL_STATE_PRESSED) {
    CAN_TRANSMIT_LIGHTS(SYSTEM_CAN_MESSAGE_LIGHTS, EE_LIGHT_STATE_ON);
  }
}

// Callback function for soft timer
// Checks whether or not a CAN message should be transmitted
static void prv_update_brake_lights(SoftTimerId timer_id, void *context) {
  PedalState current_state = get_pedal_state();
  if (current_state != s_prev_pedal_state) {
    prv_can_transmit_brake_light_change(current_state);
  }
  status_ok_or_return(soft_timer_start_millis(PEDAL_STATE_UPDATE_FREQUENCY_MS,
                                              prv_update_brake_lights, NULL, NULL));
}

StatusCode brake_light_control_init(void) {
  status_ok_or_return(soft_timer_start_millis(PEDAL_STATE_UPDATE_FREQUENCY_MS,
                                              prv_update_brake_lights, NULL, NULL));
  return STATUS_CODE_OK;
}
