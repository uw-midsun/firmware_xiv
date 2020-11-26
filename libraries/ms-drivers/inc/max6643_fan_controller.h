#pragma once
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

typedef struct {
  GpioAddress fanfail_pin;
  GpioAddress overtemp_pin;
  GpioItCallback fanfail_callback;
  GpioItCallback overtemp_callback;
  void *fanfail_callback_context;
  void *overtemp_callback_context;
} Max6643Storage;

StatusCode max6643_init(Max6643Storage *storage, Max6643Settings *settings);
