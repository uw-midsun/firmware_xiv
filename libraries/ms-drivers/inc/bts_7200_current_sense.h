#pragma once

// Reads current values from 2 ADC pins on the BTS 7200 switched between with a selection pin.
// Requires GPIO, interrupts, soft timers, and ADC to be initialized in ADC_MODE_SINGLE.
// If using with MCP23008, requires I2C to be initialized.

#include "adc.h"
#include "gpio.h"
#include "pca9539r_gpio_expander.h"
#include "soft_timer.h"
//#include "delay.h"

// Current provided at IS pin during fault conditions.
// Max input current where k(ILIS) operates is 1.44 A (see p.g. 48 of datasheet);
// with current sense ratio of k(ILIS) = 670, these values should never overlap.
#define BTS7200_FAULT_CURRENT_MIN_UA 4400
#define BTS7200_FAULT_CURRENT_MAX_UA 10000

// Max possible delay after input pin pulled low on fault, + 10 ms for buffer
#define BTS7200_FAULT_RESTART_DELAY_MS 110
#define BTS7200_FAULT_RESTART_DELAY_US (BTS7200_FAULT_RESTART_DELAY_MS * 1000)

// Check if measurement is in the range representing a fault.
#define BTS7200_IS_MEASUREMENT_FAULT(meas) \
  ((meas > BTS7200_FAULT_CURRENT_MIN_UA) && (meas < BTS7200_FAULT_CURRENT_MAX_UA))

typedef void (*Bts7200DataCallback)(uint16_t reading_out_0, uint16_t reading_out_1, void *context);

typedef void (*Bts7200FaultCallback)(bool fault0, bool fault1, void *context);

// Use when the select pin is an STM32 GPIO pin
typedef struct {
  GpioAddress *select_pin;
  GpioAddress *sense_pin;
  GpioAddress *input_0_pin;
  GpioAddress *input_1_pin;
  uint32_t interval_us;
  Bts7200DataCallback callback;  // set to NULL for no callback
  void *callback_context;
  Bts7200FaultCallback fault_callback;
  void *fault_callback_context;
} Bts7200Stm32Settings;

// Use when the select pin is through a PCA9539R GPIO expander
typedef struct {
  I2CPort i2c_port;
  Pca9539rGpioAddress *select_pin;
  GpioAddress *sense_pin;
  Pca9539rGpioAddress *input_0_pin;
  Pca9539rGpioAddress *input_1_pin;
  uint32_t interval_us;
  Bts7200DataCallback callback;  // set to NULL for no callback
  void *callback_context;
  Bts7200FaultCallback fault_callback;
  void *fault_callback_context;
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
  GpioAddress *input_0_pin_stm32;
  GpioAddress *input_1_pin_stm32;
  Pca9539rGpioAddress *input_0_pin_pca9539r;
  Pca9539rGpioAddress *input_1_pin_pca9539r;
  uint32_t interval_us;
  SoftTimerId timer_id;
  Bts7200DataCallback callback;
  void *callback_context;
  Bts7200FaultCallback fault_callback;
  void *fault_callback_context;
} Bts7200Storage;

typedef enum {
  BTS7200_FAULT_INPUT_0_TIMER = 0, 
  BTS7200_FAULT_INPUT_1_TIMER, 
  NUM_BTS7200_FAULT_TIMERS, 
} Bts7200FaultTimer;

// Initialize the BTS7200 with the given settings; the select pin is an STM32 GPIO pin.
StatusCode bts_7200_init_stm32(Bts7200Storage *storage, Bts7200Stm32Settings *settings);

// Initialize the BTS7200 with the given settings; the select pin is through a PCA9539R.
StatusCode bts_7200_init_pca9539r(Bts7200Storage *storage, Bts7200Pca9539rSettings *settings);

// Read the latest measurements. This does not get measurements from the storage but instead
// reads them from the BTS7200 itself.
StatusCode bts_7200_get_measurement(Bts7200Storage *storage, uint16_t *meas0, uint16_t *meas1);

// Enable output 0 by pulling the IN0 pin high.
StatusCode bts_7200_enable_output_0(Bts7200Storage *storage);

// Disable output 0 by pulling the IN0 pin low.
StatusCode bts_7200_disable_output_0(Bts7200Storage *storage);

// Enable output 1 by pulling the IN1 pin high.
StatusCode bts_7200_enable_output_1(Bts7200Storage *storage);

// Disable output 1 by pulling the IN1 pin low.
StatusCode bts_7200_disable_output_1(Bts7200Storage *storage);

// Return whether output 0 is enabled or disabled
bool bts_7200_get_output_0_enabled(Bts7200Storage *storage);

// Return whether output 1 is enabled or disabled
bool bts_7200_get_output_1_enabled(Bts7200Storage *storage);

// Set up a soft timer which periodically updates the storage with the latest measurements.
// DO NOT USE if you are reading with bts_7200_get_measurement.
StatusCode bts_7200_start(Bts7200Storage *storage);

// Stop the timer associated with the storage and return whether it was successful.
bool bts_7200_stop(Bts7200Storage *storage);
