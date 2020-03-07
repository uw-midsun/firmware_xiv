#include "stop_charger.h"
#include "charger_events.h"

#include "can.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "gpio.h"

static GpioAddress s_control_pilot_pin = { .port = GPIO_PORT_A, .pin = 2 };

void stop_charger(void) {
  // TODO(SOFT-130): deactivate charger controller
  // set control pilot pin
  gpio_set_state(&s_control_pilot_pin, GPIO_STATE_LOW);
  // TODO(SOFT-130): broadcast charger disconnected can message
}

void stop_charger_process_event(const Event *e) {
  if (e->id == CHARGER_STOP_EVENT_STOP) {
    stop_charger();
  }
}
