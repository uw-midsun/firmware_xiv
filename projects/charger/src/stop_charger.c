#include "stop_charger.h"
#include "charger_events.h"
#include "charger_controller.h"

#include "can.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "gpio.h"
#include "exported_enums.h"

static GpioAddress s_control_pilot_pin = { .port = GPIO_PORT_A, .pin = 2 };

void stop_charger(void) {
  // deactivate charger controller
  charger_controller_deactivate();
  // set control pilot pin
  gpio_set_state(&s_control_pilot_pin, GPIO_STATE_LOW);
  // broadcast charger disconnected can message
  CAN_TRANSMIT_CHARGER_CONNECTED_STATE(EE_CHARGER_CONN_STATE_DISCONNECTED);
}

void stop_charger_process_event(const Event *e) {
  if (e->id == CHARGER_STOP_EVENT_STOP) {
    stop_charger();
  }
}
