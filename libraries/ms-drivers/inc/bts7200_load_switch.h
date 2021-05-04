#pragma once
// Driver for the BTS7200-2EPA load switch.

// Reads current values from 2 ADC pins on the BTS7200 switched between with a selection pin.
// Requires GPIO, interrupts, soft timers, and ADC to be initialized in ADC_MODE_SINGLE.
// If using with PCA9539R, requires I2C to be initialized.

// Due to the fault handling procedures for the BTS7200, all BTS7200 pins should
// only be manipulated through this driver and bts7xxx_common.

#include "adc.h"
#include "bts7xxx_common.h"
#include "bts7xxx_common_impl.h"
#include "gpio.h"
#include "pca9539r_gpio_expander.h"
#include "soft_timer.h"
#include "status.h"

// Upper maximum for the possible leakage voltage that may be read from the SENSE pin at
// T(env) < 80 C (see p.g. 27 of BTS7200 datasheet)
#define BTS7200_MAX_LEAKAGE_VOLTAGE_MV 2

// Nominal scaling factor k(ILIS) for current output at the SENSE pin in normal operation.
#define BTS7200_IS_SCALING_NOMINAL 670

// Max possible delay after input pin pulled low on fault, + 10 ms for buffer
#define BTS7200_FAULT_RESTART_DELAY_MS BTS7XXX_FAULT_RESTART_DELAY_MS
#define BTS7200_FAULT_RESTART_DELAY_US (BTS7200_FAULT_RESTART_DELAY_MS * 1000)

typedef enum {
  BTS7200_CHANNEL_0 = 0,
  BTS7200_CHANNEL_1,
  NUM_BTS7200_CHANNELS,
} Bts7200Channel;

typedef void (*Bts7200DataCallback)(uint16_t reading_out_0, uint16_t reading_out_1, void *context);

// If provided, this callback will be called when there's a protection event on faulting_channel.
// See section 8.3 (p. 32) of BTS7200 datasheet.
typedef void (*Bts7200FaultCallback)(Bts7200Channel faulting_channel, void *context);

// Use when the select pin is an STM32 GPIO pin
typedef struct Bts7200Stm32Settings {
  GpioAddress *select_pin;
  GpioAddress *sense_pin;
  GpioAddress *enable_0_pin;
  GpioAddress *enable_1_pin;
  uint32_t interval_us;
  Bts7200DataCallback callback;  // set to NULL for no callback
  void *callback_context;
  Bts7200FaultCallback fault_callback;
  void *fault_callback_context;
  uint32_t resistor;  // resistor value (in ohms) used to convert SENSE voltage to current
  int32_t bias;       // experimental bias to be subtracted from the resulting current, in mA
  // Faults are indicated by high voltage on the sense pin, see section 8.3 of BTS7200 datasheet.
  uint16_t min_fault_voltage_mv;  // min voltage representing a fault, in mV
} Bts7200Stm32Settings;

// Use when the select pin is through a PCA9539R GPIO expander
typedef struct Bts7200Pca9539rSettings {
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
  uint32_t resistor;  // resistor value (in ohms) used to convert SENSE voltage to current
  int32_t bias;       // experimental bias to be subtracted from the resulting current, in mA
  // Faults are indicated by high voltage on the sense pin, see section 8.3 of BTS7200 datasheet.
  uint16_t min_fault_voltage_mv;  // min voltage representing a fault, in mV
} Bts7200Pca9539rSettings;

typedef struct Bts7200Storage {
  uint16_t reading_out_0;  // Reading from IN0, in mA
  uint16_t reading_out_1;  // Reading from IN1, in mA
  Bts7xxxSelectPin select_pin;
  GpioAddress *sense_pin;
  Bts7xxxEnablePin enable_pin_0;
  Bts7xxxEnablePin enable_pin_1;
  uint32_t interval_us;
  SoftTimerId measurement_timer_id;
  Bts7200DataCallback callback;
  void *callback_context;
  Bts7200FaultCallback fault_callback;
  void *fault_callback_context;
  uint32_t resistor;  // resistor value (in ohms) used to convert SENSE voltage to current
  int32_t bias;       // experimental bias to be subtracted from the resulting current, in mA
  uint16_t min_fault_voltage_mv;  // min voltage representing a fault, in mV
} Bts7200Storage;

// Initialize the BTS7200 with the given settings; the select and enable
// pins are STM32 GPIO pins.
StatusCode bts7200_init_stm32(Bts7200Storage *storage, Bts7200Stm32Settings *settings);

// Initialize the BTS7200 with the given settings; the select pin and enable
// pins are through a PCA9539R.
StatusCode bts7200_init_pca9539r(Bts7200Storage *storage, Bts7200Pca9539rSettings *settings);

// Read the latest input current measurement from the given channel, in mA. This does not get
// measurements from the storage but instead reads them from the BTS7200 itself. Note that, due to
// the fault handling implementation, the pointer to storage has to be valid for
// BTS7200_FAULT_RESTART_DELAY_MS. Otherwise, bts7200_stop must be called before the pointer is
// freed to prevent segfaults.
StatusCode bts7200_get_measurement(Bts7200Storage *storage, uint16_t *meas, Bts7200Channel channel);

// Enable output 0.
StatusCode bts7200_enable_output_0(Bts7200Storage *storage);

// Disable output 0.
StatusCode bts7200_disable_output_0(Bts7200Storage *storage);

// Enable output 1.
StatusCode bts7200_enable_output_1(Bts7200Storage *storage);

// Disable output 1.
StatusCode bts7200_disable_output_1(Bts7200Storage *storage);

// Return whether output 0 is enabled or disabled.
bool bts7200_get_output_0_enabled(Bts7200Storage *storage);

// Return whether output 1 is enabled or disabled.
bool bts7200_get_output_1_enabled(Bts7200Storage *storage);

// Set up a soft timer which periodically updates the storage with the latest measurements.
// DO NOT USE if you are reading with bts7200_get_measurement.
StatusCode bts7200_start(Bts7200Storage *storage);

// Stop the timer and any active fault timers associated with the storage.
void bts7200_stop(Bts7200Storage *storage);
