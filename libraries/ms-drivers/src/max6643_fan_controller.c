#include <stddef.h>
#include <string.h>

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "max6643_fan_controller.h"

StatusCode max6643_init(Max6643Storage *storage, Max6643Settings *settings) {
  if (storage == NULL || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(storage, 0, sizeof(Max6643Storage));

  storage->overtemp_pin = settings->overtemp_pin;
  storage->fanfail_pin = settings->fanfail_pin;
  storage->overtemp_callback = settings->overtemp_callback;
  storage->fanfail_callback = settings->fanfail_callback;
  storage->fanfail_callback_context = settings->fanfail_callback_context;
  storage->overtemp_callback_context = settings->overtemp_callback_context;

  static InterruptSettings s_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_it_register_interrupt(&storage->overtemp_pin, &s_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             storage->overtemp_callback, storage->overtemp_callback_context);

  gpio_it_register_interrupt(&storage->fanfail_pin, &s_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             storage->fanfail_callback, storage->fanfail_callback_context);

  return STATUS_CODE_OK;
}
