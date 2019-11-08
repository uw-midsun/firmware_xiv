#pragma once
// The debouncer module which provides the function
//   for debouncing a GPIO input pin.
// Debouncing is simply a procedure to handle the physical "bouncing" of a
// switch
//   once it is pushed or released. The problem with bouncing is that it
//   creates unwanted (extra) signals which raise interrupts and prevent the
//   main code from executing.
// Requires GPIO, interrupts and soft timer to be initialized.
#include "gpio.h"
#include "gpio_it.h"
#include "status.h"

typedef struct DebouncerStorage {
  GpioAddress address;
  GpioState state;
  GpioItCallback callback;
  void *context;
} DebouncerStorage;

// Inits the GPIO input pin and sets up the debouncer for it.
// debouncer_info is a storage created by the user, and it should persist.
StatusCode debouncer_init_pin(DebouncerStorage *debouncer, const GpioAddress *address,
                              GpioItCallback callback, void *context);
