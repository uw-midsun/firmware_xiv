#include "smoke_uv_cutoff.h"
#include "bug.h"
#include "front_uv_detector.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "pd_events.h"
#include "pin_defs.h"
#include "wait.h"

void smoke_uv_cutoff_perform(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  event_queue_init();

  BUG(front_uv_detector_init(&(GpioAddress)FRONT_UV_COMPARATOR_PIN));
  LOG_DEBUG("The UV cutoff smoke test has been successfully initialized.");

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
    }
    wait();
  }
}
