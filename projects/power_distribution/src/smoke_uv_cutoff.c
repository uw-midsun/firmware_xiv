#include "bug.h"
#include "can_transmit.h"
#include "front_uv_detector.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "pd_events.h"
#include "pin_defs.h"
#include "status.h"
#include "wait.h"

void smoke_uv_cutoff_perform(void) {
  gpio_init();
  gpio_it_init();
  event_queue_init();
  interrupt_init();

  static void prv_pin_interrupt_handler(const GpioAddress *address, void *context) {
    CAN_TRANSMIT_UV_CUTOFF_NOTIFICATION();

    LOG_WARN("UV lockout was triggered\n");
  }

  StatusCode front_uv_detector_init(GpioAddress * detector_pin) {
    InterruptSettings interrupt_settings = {
      .type = INTERRUPT_TYPE_INTERRUPT,
      .priority = INTERRUPT_PRIORITY_NORMAL,
    };

    // callback occurs when pin turns low i.e falling
    status_ok_or_return(gpio_it_register_interrupt(detector_pin, &interrupt_settings,
                                                   INTERRUPT_EDGE_FALLING,
                                                   prv_pin_interrupt_handler, NULL));

    return STATUS_CODE_OK;
  }

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
      pd_gpio_process_event(&e);
    }
    wait();
  }
}
