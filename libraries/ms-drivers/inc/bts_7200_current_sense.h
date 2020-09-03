#pragma once

// Reads current values from 2 ADC pins on the BTS 7200 switched between with a selection pin.
// Requires GPIO, interrupts, soft timers, and ADC to be initialized in ADC_MODE_SINGLE.
// If using with PCA9539R, requires I2C to be initialized.

#include "adc.h"
#include "gpio.h"
#include "pca9539r_gpio_expander.h"
#include "soft_timer.h"

typedef void (*Bts7200DataCallback)(uint16_t reading_out_0, uint16_t reading_out_1, void *context);

// Use when the select pin is an STM32 GPIO pin
typedef struct {
  GpioAddress *select_pin;
  GpioAddress *sense_pin;
  uint32_t interval_us;
  Bts7200DataCallback callback;  // set to NULL for no callback
  void *callback_context;
} Bts7200Stm32Settings;

// Use when the select pin is through a PCA9539R GPIO expander
typedef struct {
  I2CPort i2c_port;
  Pca9539rGpioAddress *select_pin;
  GpioAddress *sense_pin;
  uint32_t interval_us;
  Bts7200DataCallback callback;  // set to NULL for no callback
  void *callback_context;
} Bts7200Pca9539rSettings;

typedef enum {
  BTS7200_SELECT_PIN_STM32 = 0,
  BTS7200_SELECT_PIN_PCA9539R,
  NUM_BTS7200_SELECT_PINS,
} Bts7200SelectPinType;

typedef struct {
  uint16_t reading_out_0;
  uint16_t reading_out_1;
  GpioAddress *select_pin_stm32;
  Pca9539rGpioAddress *select_pin_pca9539r;
  Bts7200SelectPinType select_pin_type;
  GpioAddress *sense_pin;
  uint32_t interval_us;
  SoftTimerId timer_id;
  Bts7200DataCallback callback;
  void *callback_context;
} Bts7200Storage;

// Initialize the BTS7200 with the given settings; the select pin is an STM32 GPIO pin.
StatusCode bts_7200_init_stm32(Bts7200Storage *storage, Bts7200Stm32Settings *settings);

// Initialize the BTS7200 with the given settings; the select pin is through a PCA9539R.
StatusCode bts_7200_init_pca9539r(Bts7200Storage *storage, Bts7200Pca9539rSettings *settings);

// Read the latest measurements. This does not get measurements from the storage but instead
// reads them from the BTS7200 itself.
StatusCode bts_7200_get_measurement(Bts7200Storage *storage, uint16_t *meas0, uint16_t *meas1);

// quick-n-dirty hack, to do: make it good
// Get the measurements on the callback provided. It is NOT SAFE to call this again before the
// callback is called.
void bts_7200_get_measurement_with_delay(Bts7200Storage *storage);

// Set up a soft timer which periodically updates the storage with the latest measurements.
// DO NOT USE if you are reading with bts_7200_get_measurement.
StatusCode bts_7200_start(Bts7200Storage *storage);

// Stop the timer associated with the storage and return whether it was successful.
bool bts_7200_stop(Bts7200Storage *storage);
