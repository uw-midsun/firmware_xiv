#include "max6643_fan_controller.h"

#include <stddef.h>
#include <string.h>

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"

StatusCode max6643_init(Max6643Settings *settings) {
  if (settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  InterruptSettings s_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_it_register_interrupt(&settings->overtemp_pin, &s_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             settings->overtemp_callback, settings->overtemp_callback_context);

  gpio_it_register_interrupt(&settings->fanfail_pin, &s_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             settings->fanfail_callback, settings->fanfail_callback_context);

  return STATUS_CODE_OK;
}
