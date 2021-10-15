#include "smoke_charger_comms.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "charger_controller.h"
#include "event_queue.h"
#include "gpio.h"
#include "log.h"
#include "soft_timer.h"

#define COUNTER_PERIOD_MS 10000  // The time between each softtimer call in ms

static void prv_softtimer_charger_controller_call(SoftTimerId timer_id, void *context) {
  charger_controller_active();
  charger_controller_deactive();
  soft_timer_start_millis(COUNTER_PERIOD_MS, prv_softtimer_charger_controller_call, NULL, NULL);
}

void smoke_charger_controll_perform(void) {
  gpio_init();
  soft_timer_init();
  event_queue_init();
  can_init();
  charger_controller_init();
  while (true) {
    {
      soft_timer_start_millis(COUNTER_PERIOD_MS, prv_softtimer_charger_controller_call, NULL, NULL);
      LOG_DEBUG("Initializing soft timer for charger controller smoke test\n");
    }
    wait();
  }
}
