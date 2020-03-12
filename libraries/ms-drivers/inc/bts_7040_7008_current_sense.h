#pragma once
// Driver for the BTS7040-1EPA and BTS7008-1EPP (and probably more) input switches.
// Requires GPIO and ADC (in ADC_MODE_SINGLE) to be initialized.
#include "gpio.h"

typedef struct {
  GpioAddress *sense_pin;
} Bts7040Settings;

typedef struct {
  GpioAddress *sense_pin;
} Bts7040Storage;

StatusCode bts_7040_init(Bts7040Storage *storage, Bts7040Settings *settings);

StatusCode bts_7040_get_measurement(Bts7040Storage *storage, uint16_t *measured);
