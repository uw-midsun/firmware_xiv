#pragma once
// Driver for the BTS7040-1EPA load switch.
// Requires GPIO, interrupts, soft timers, and ADC (in ADC_MODE_SINGLE) to be initialized.

// If using with PCA9539R, required I2C to be initialized.

// Due to fault handling procedures for the BTS7040, all BTS7040 pins should only be manipulated
// through this driver and bts7xxx_common.

#include "adc.h"
#include "bts7xxx_common.h"
#include "bts7xxx_common_impl.h"
#include "gpio.h"
#include "pca9539r_gpio_expander.h"
#include "soft_timer.h"

// Upper maximum for the possible leakage voltage that may be read from the SENSE pin at
// T(env) < 80 C (see p.g. 27 of BTS7040 datasheet)
#define BTS7040_MAX_LEAKAGE_VOLTAGE_MV 2

// Nominal scaling factor k(ILIS) for current output at the SENSE pin in normal operation.
// This is the average of two nominal values given: 1750 with I(load) <= 0.25 A,
// and 1800 with I(load) >= 1A.
#define BTS7040_IS_SCALING_NOMINAL 1775

#define BTS7004_IS_SCALING_NOMINAL 20000

// Max possible delay after input pin pulled low on fault, + 10 ms for buffer
// (see p.g. 39 of datasheet)
#define BTS7040_FAULT_RESTART_DELAY_MS BTS7XXX_FAULT_RESTART_DELAY_MS
#define BTS7040_FAULT_RESTART_DELAY_US (BTS7040_FAULT_RESTART_DELAY_MS * 1000)

typedef void (*Bts7040DataCallback)(uint16_t reading_out, void *context);

// Called in case of overtemperature or overvoltage, see section 8.3 of BTS7040 datasheet.
typedef void (*Bts7040FaultCallback)(void *context);

// Use when the select pin is an STM32 GPIO pin
typedef struct {
  GpioAddress *sense_pin;
  GpioAddress *enable_pin;
  uint32_t interval_us;
  Bts7040DataCallback callback;  // set to NULL for no callback
  void *callback_context;
  Bts7040FaultCallback fault_callback;
  void *fault_callback_context;
  uint32_t resistor;  // resistor value (in ohms) used to convert SENSE voltage to current
  int32_t bias;       // experimental bias to be subtracted from the resulting current, in mA
  // Faults are indicated by high voltage on the sense pin, see section 8.3 of BTS7200 datasheet.
  uint16_t min_fault_voltage_mv;  // min voltage representing a fault, in mV
  bool use_bts7004_scaling;
} Bts7040Stm32Settings;

// Use when the select pin is through a PCA9539R GPIO expander
typedef struct {
  I2CPort i2c_port;
  GpioAddress *sense_pin;
  Pca9539rGpioAddress *enable_pin;
  uint32_t interval_us;
  Bts7040DataCallback callback;  // set to NULL for no callback
  void *callback_context;
  Bts7040FaultCallback fault_callback;
  void *fault_callback_context;
  uint32_t resistor;  // resistor value (in ohms) used to convert SENSE voltage to current
  int32_t bias;       // experimental bias to be subtracted from the resulting current, in mA
  // Faults are indicated by high voltage on the sense pin, see section 8.3 of BTS7200 datasheet.
  uint16_t min_fault_voltage_mv;  // min voltage representing a fault, in mV
  bool use_bts7004_scaling;
} Bts7040Pca9539rSettings;

typedef struct {
  uint16_t reading_out;  // Reading from IN pin, in mA
  GpioAddress *sense_pin;
  Bts7xxxEnablePin enable_pin;
  uint32_t interval_us;
  SoftTimerId measurement_timer_id;
  Bts7040DataCallback callback;
  void *callback_context;
  Bts7040FaultCallback fault_callback;
  void *fault_callback_context;
  uint32_t resistor;  // resistor value (in ohms) used to convert SENSE voltage to current
  int32_t bias;       // experimental bias to be subtracted from the resulting current, in mA
  uint16_t min_fault_voltage_mv;  // min voltage representing a fault, in mV
  bool use_bts7004_scaling;
} Bts7040Storage;

// Initialize the BTS7040 with the given settings; the enable pin is a STM32 GPIO pins.
StatusCode bts7040_init_stm32(Bts7040Storage *storage, Bts7040Stm32Settings *settings);

// Initialize the BTS7040 with the given settings; the enable pin is through a PCA9539R.
StatusCode bts7040_init_pca9539r(Bts7040Storage *storage, Bts7040Pca9539rSettings *settings);

// Enable output by pulling the IN pin high.
StatusCode bts7040_enable_output(Bts7040Storage *storage);

// Disable output by pulling the IN pin low.
StatusCode bts7040_disable_output(Bts7040Storage *storage);

// Returns whether the output is enabled or disabled.
bool bts7040_get_output_enabled(Bts7040Storage *storage);

// Read the latest current input current measurement, in mA. This does not get the measurement from
// the storage but instead reads it from the BTS7040 itself. Note that, due to the fault handling
// implementation, the pointer to storage has to be valid for BTS7040_FAULT_RESTART_DELAY_MS.
// Otherwise, bts7040_stop must be called before the pointer is freed to prevent segfaults.
StatusCode bts7040_get_measurement(Bts7040Storage *storage, uint16_t *meas);

// Set up a soft timer which periodically updates the storage with the latest measurement.
// DO NOT USE if you are reading with bts7040_get_measurement.
StatusCode bts7040_start(Bts7040Storage *storage);

// Stop the measurement and fault timers associated with the storage.
void bts7040_stop(Bts7040Storage *storage);
