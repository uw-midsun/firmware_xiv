// uv_comp PB0

// Checks if UV cutoff has occurred and sends message to telemetry.

#include "adc.h"
#include "can.h"
#include "can_msg.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "pd_events.h"
#include "pin_defs.h"
#include "soft_timer.h"

static GpioAddress s_uv_comp_pin_address = { .port = GPIO_PORT_B, .pin = 0 };

static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL,
};

static void prv_pin_interrupt_handler(const GpioAddress *address, void *context) {
  CAN_TRANSMIT_UV_CUTOFF_NOTIFICATION();

  LOG_WARN("UV lockout was triggered\n");
}

void prv_register_interrupts(void) {
  // callback occurs when pin turns low i.e falling
  gpio_it_register_interrupt(&s_uv_comp_pin_address, &s_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_pin_interrupt_handler, NULL);
}

StatusCode front_uv_detector_init() {
  // check if all are needed
  // raiyan todo wrap in status ok or return
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();
  prv_register_interrupts();

  return STATUS_CODE_OK;
}
