#include "stop_sequence.h"

#include "charger_controller.h"
#include "charger_defs.h"
#include "charger_events.h"
#include "event_queue.h"
#include "gpio.h"

static const GpioAddress s_cp_select = CONTROL_PILOT_SEL_PIN;
static const GpioAddress s_relay_en = RELAY_EN_PIN;
static const GpioAddress s_load_sw_en = LOAD_SW_EN_PIN;

static void prv_stop_sequence() {
  // 1. Deactivate charger
  charger_controller_deactivate();

  // 2. Turn off control pilot
  gpio_set_state(&s_cp_select, GPIO_STATE_LOW);

  // 3. Turn off load switch and relay
  gpio_set_state(&s_load_sw_en, GPIO_STATE_LOW);
  gpio_set_state(&s_relay_en, GPIO_STATE_LOW);
}

void stop_sequence_process_event(const Event *e) {
  if (e->id == CHARGER_CHARGE_EVENT_STOP) {
    prv_stop_sequence();
  }
}
