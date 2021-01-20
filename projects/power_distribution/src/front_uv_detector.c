#include "front_uv_detector.h"

#include "can_transmit.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"

static GpioAddress s_uv_comp_pin_address = { .port = GPIO_PORT_B, .pin = 0 };

static void prv_pin_interrupt_handler(const GpioAddress *address, void *context) {
  CAN_TRANSMIT_UV_CUTOFF_NOTIFICATION();

  LOG_WARN("UV lockout was triggered\n");
}

StatusCode front_uv_detector_init(GpioAddress *detector_pin) {
  interrupt_init();
  status_ok_or_return(gpio_init());
  gpio_it_init();

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  // callback occurs when pin turns low i.e falling
  status_ok_or_return(gpio_it_register_interrupt(
      detector_pin, &interrupt_settings, INTERRUPT_EDGE_FALLING, prv_pin_interrupt_handler, NULL));

  return STATUS_CODE_OK;
}
