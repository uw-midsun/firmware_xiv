#include "can_transmit.h"
#include "front_uv_detector.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "pd_events.h"
#include "pin_defs.h"
#include "status.h"
#include "wait.h"

void smoke_uv_cutoff_perform(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  event_queue_init();

  // Initialize module for UV cutoff notification
  StatusCode front_uv_detector_init(GpioAddress * detector_pin);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
      pd_gpio_process_event(&e);
      lights_signal_fsm_process_event(&s_lights_signal_fsm_storage, &e);
      if (!is_front_pd) {
        rear_strobe_blinker_process_event(&e);
      }
    }
    wait();
  }
}
