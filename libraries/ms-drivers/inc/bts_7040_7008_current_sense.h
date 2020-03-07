#pragma once
// Driver for the BTS7040-1EPA and BTS7008-1EPP (and probably more) input switches.
// To be used as configured on the power distribution boards, behind PCA9539R and 4-bit mux.
// Requires I2C, GPIO, and ADC (in ADC_MODE_SINGLE) to be initialized and the PCA9539Rs
// corresponding to given pins to be initalized.
#include "gpio.h"
#include "pca9539r_gpio_expander.h"

typedef struct {
  Pca9539rGpioAddress *enable_pin;
  GpioAddress *sense_pin;
} Bts7040Settings;

typedef struct {
  GpioAddress *sense_pin;
} Bts7040Storage;

StatusCode bts_7040_init(Bts7040Storage *storage, Bts7040Settings *settings);

StatusCode bts_7040_get_measurement(Bts7040Storage *storage, uint16_t *measured);
