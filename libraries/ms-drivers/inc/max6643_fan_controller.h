#pragma once
// Driver for the MAX6643 fan controller
// Requires only initialization since part is driven fully automatically
// High and low temperature thresholds, as well as full speed pins are set in hardware
// Requires interrupts, GPIO, and GPIO interrupts to be initialized

#include "gpio.h"
#include "gpio_it.h"

typedef struct {
  GpioAddress fanfail_pin;
  GpioAddress overtemp_pin;
  GpioItCallback fanfail_callback;
  GpioItCallback overtemp_callback;  // set to NULL for no callback
  void *fanfail_callback_context;
  void *overtemp_callback_context;
} Max6643Settings;

// typedef struct {
//   GpioAddress fanfail_pin;
//   GpioAddress overtemp_pin;
//   GpioItCallback fanfail_callback;
//   GpioItCallback overtemp_callback;
//   void *fanfail_callback_context;
//   void *overtemp_callback_context;
// } Max6643Storage;

StatusCode max6643_init(Max6643Settings *settings);
