#pragma once
// Common definitions for BTS7XXX-series load switches. 
// Most of the functions in this file allow for a common API to easily interact with the 
// EN/SEL pins on the load switches, abstracting the specific method oif accessing the pin.

#include "gpio.h"
#include "pca9539r_gpio_expander.h"
#include "soft_timer.h"

// Represents whether SEL/EN pins are accessed through STM32 or a Pca9539R
typedef enum {
  BTS7XXX_PIN_STM32 = 0,
  BTS7XXX_PIN_PCA9539R,
  NUM_BTS7XXX_PIN_TYPES,
} Bts7xxxPinType;

// Holds pin-specific info for EN pins
typedef struct {
  GpioAddress *enable_pin_stm32;
  Pca9539rGpioAddress *enable_pin_pca9539r;
  SoftTimerId fault_timer_id;
  bool fault_in_progress;
  Bts7xxxPinType pin_type;
} Bts7xxxEnablePin;

// Holds pin-specific info for SEL pins
typedef struct {
  GpioAddress *select_pin_stm32;
  Pca9539rGpioAddress *select_pin_pca9539r;
  Bts7xxxPinType pin_type;
} Bts7xxxSelectPin;

// Broad function to enable the pin passed in.
StatusCode prv_bts_7xxx_enable_pin(Bts7xxxEnablePin *pin);

// Broad function to disable the pin passed in.
StatusCode prv_bts7xxx_disable_pin(Bts7xxxEnablePin *pin);

// Broad function to get whether the pin passed in is enabled.
StatusCode prv_bts7xxx_get_pin_enabled(Bts7xxxEnablePin *pin);

// Broad pin soft timer cb without re-enabling the pin
void prv_bts7xxx_fault_handler_cb(SoftTimerId timer_id, void *context);

// Broad pin re-enable soft timer cb
void prv_bts7xxx_fault_handler_enable_cb(SoftTimerId timer_id, void *context);

// Helper function to clear fault on a given pin
StatusCode prv_bts7xxx_handle_fault_pin(Bts7xxxEnablePin *pin);