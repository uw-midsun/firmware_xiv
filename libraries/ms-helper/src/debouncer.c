#include "debouncer.h"
// This module is the debouncer for gpio.
// First an interrupt is registerred on the pin with prv_it_callback.
// From there, the callback saves the state, masks the interrupt and creates a
// soft timer. Once the timer times out, prv_timer_callback is called
//   which compares the states and runs the users callback if required.
//   Interrupt is reset after.
#include <stddef.h>
#include "soft_timer.h"

#define DEBOUNCER_INTERRUPT_MASKING_DURATION_MS 50

// This is the callback for the soft timer. If there is a button input, it runs
// the user's callback.
static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  DebouncerStorage *debouncer = context;

  GpioState current_state;
  gpio_get_state(&debouncer->address, &current_state);
  if (debouncer->callback && current_state == debouncer->state) {
    debouncer->callback(&debouncer->address, debouncer->context);
  }
  gpio_it_mask_interrupt(&debouncer->address, false);
}

// This is the interrupt callback to start off the debouncing
static void prv_it_callback(const GpioAddress *address, void *context) {
  DebouncerStorage *debouncer = context;
  gpio_get_state(address, &debouncer->state);

  gpio_it_mask_interrupt(address, true);

  soft_timer_start_millis(DEBOUNCER_INTERRUPT_MASKING_DURATION_MS, prv_timer_callback, debouncer,
                          NULL);
}

StatusCode debouncer_init_pin(DebouncerStorage *debouncer, const GpioAddress *address,
                              GpioItCallback callback, void *context) {
  GpioSettings gpio_settings = {
    .direction = GPIO_DIR_IN,   //
    .resistor = GPIO_RES_NONE,  //
  };

  gpio_init_pin(address, &gpio_settings);

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_LOW,
  };

  debouncer->address = *address;
  debouncer->callback = callback;
  debouncer->context = context;

  return gpio_it_register_interrupt(address, &interrupt_settings, INTERRUPT_EDGE_RISING_FALLING,
                                    prv_it_callback, debouncer);
}
