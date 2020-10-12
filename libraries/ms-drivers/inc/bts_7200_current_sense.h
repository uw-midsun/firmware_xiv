#pragma once

// Reads current values from 2 ADC pins on the BTS 7200 switched between with a selection pin.
// Requires GPIO, interrupts, soft timers, and ADC to be initialized in ADC_MODE_SINGLE.
// If using with MCP23008, requires I2C to be initialized.

// Due to the fault handling procedures for the BTS7200, all BTS7200 pins should
// only be manipulated through this driver.

#include "adc.h"
#include "gpio.h"
#include "pca9539r_gpio_expander.h"
#include "soft_timer.h"

// Upper maximum for the possible leakage voltage that may be read from the SENSE pin at
// T(env) < 80 C (see p.g. 27 of BTS7200 datasheet)
#define BTS7200_MAX_LEAKAGE_VOLTAGE_MV 2

// Nominal scaling factor k(ILIS) for current output at the SENSE pin in normal operation.
#define BTS7200_IS_SCALING_NOMINAL 670

// Voltage at the SENSE pin is limited to a max of 3.3V by a diode.
// Due to to this function, since any fault current will be at least 4.4 mA (see p.g. 49)
// the resulting voltage will be 4.4 mA * 1.6 kOhm = ~7 V. Due to this,
// voltages approaching 3.3V represent a fault, and should be treated as such.
#define BTS7200_MAX_VALID_SENSE_VOLTAGE_MV 3200

// Check if measurement is a fault.
#define BTS7200_IS_MEASUREMENT_FAULT_MV(meas) ((meas) > BTS7200_MAX_VALID_SENSE_VOLTAGE_MV)

// Max possible delay after input pin pulled low on fault, + 10 ms for buffer
#define BTS7200_FAULT_RESTART_DELAY_MS 110
#define BTS7200_FAULT_RESTART_DELAY_US (BTS7200_FAULT_RESTART_DELAY_MS * 1000)

typedef void (*Bts7200DataCallback)(uint16_t reading_out_0, uint16_t reading_out_1, void *context);

typedef void (*Bts7200FaultCallback)(bool fault0, bool fault1, void *context);

// Represents whether SEL/EN pins are accessed through STM32 or a Pca9539R
typedef enum {
  BTS7200_PIN_STM32 = 0,
  BTS7200_PIN_PCA9539R,
  NUM_BTS7200_PIN_TYPES,
} Bts7200PinType;

// Holds pin-specific info for EN pins
typedef struct {
  GpioAddress *enable_pin_stm32;
  Pca9539rGpioAddress *enable_pin_pca9539r;
  SoftTimerId fault_timer_id;
  bool fault_in_progress;
  Bts7200PinType pin_type;
} Bts7200EnablePin;

// Holds pin-specific info for SEL pins
typedef struct {
  GpioAddress *select_pin_stm32;
  Pca9539rGpioAddress *select_pin_pca9539r;
  Bts7200PinType pin_type;
} Bts7200SelectPin;

// Use when the select pin is an STM32 GPIO pin
typedef struct {
  GpioAddress *select_pin;
  GpioAddress *sense_pin;
  GpioAddress *enable_0_pin;
  GpioAddress *enable_1_pin;
  uint32_t interval_us;
  Bts7200DataCallback callback;  // set to NULL for no callback
  void *callback_context;
  Bts7200FaultCallback fault_callback;
  void *fault_callback_context;
  int resistor;  // scaling performed by resistor at SENSE pin
} Bts7200Stm32Settings;

// Use when the select pin is through a PCA9539R GPIO expander
typedef struct {
  I2CPort i2c_port;
  Pca9539rGpioAddress *select_pin;
  GpioAddress *sense_pin;
  Pca9539rGpioAddress *enable_0_pin;
  Pca9539rGpioAddress *enable_1_pin;
  uint32_t interval_us;
  Bts7200DataCallback callback;  // set to NULL for no callback
  void *callback_context;
  Bts7200FaultCallback fault_callback;
  void *fault_callback_context;
  int resistor;  // scaling performed by resistor at SENSE pin
} Bts7200Pca9539rSettings;

typedef struct {
  uint16_t reading_out_0;
  uint16_t reading_out_1;
  Bts7200SelectPin select_pin;
  GpioAddress *sense_pin;
  Bts7200EnablePin enable_pin_0;
  Bts7200EnablePin enable_pin_1;
  uint32_t interval_us;
  SoftTimerId measurement_timer_id;
  Bts7200DataCallback callback;
  void *callback_context;
  Bts7200FaultCallback fault_callback;
  void *fault_callback_context;
  int resistor;  // scaling performed by resistor at SENSE pin
} Bts7200Storage;

// Initialize the BTS7200 with the given settings; the select pin is an STM32 GPIO pin.
StatusCode bts_7200_init_stm32(Bts7200Storage *storage, Bts7200Stm32Settings *settings);

// Initialize the BTS7200 with the given settings; the select pin is through a PCA9539R.
StatusCode bts_7200_init_pca9539r(Bts7200Storage *storage, Bts7200Pca9539rSettings *settings);

// Read the latest measurements. This does not get measurements from the storage but instead
// reads them from the BTS7200 itself.
// Note that, due to the fault handling implementation, the pointer to storage has to be
// valid for BTS7200_FAULT_RESTART_DELAY_MS. Otherwise, bts_7200_stop must be called
// before the pointer is freed to prevent segfaults.
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
