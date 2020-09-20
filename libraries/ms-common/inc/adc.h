#pragma once
// Analog to Digital Converter HAL Inteface
// Requires GPIO and interrupts to be initialized.
#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "status.h"

typedef enum {
  ADC_MODE_SINGLE = 0,
  ADC_MODE_CONTINUOUS,
  NUM_ADC_MODES,
} AdcMode;

typedef void (*AdcPinCallback)(GpioAddress address, void *context);

// Following enum deprecated in favour of GpioAddress Version
typedef enum {
  ADC_CHANNEL_0 = 0,
  ADC_CHANNEL_1,
  ADC_CHANNEL_2,
  ADC_CHANNEL_3,
  ADC_CHANNEL_4,
  ADC_CHANNEL_5,
  ADC_CHANNEL_6,
  ADC_CHANNEL_7,
  ADC_CHANNEL_8,
  ADC_CHANNEL_9,
  ADC_CHANNEL_10,
  ADC_CHANNEL_11,
  ADC_CHANNEL_12,
  ADC_CHANNEL_13,
  ADC_CHANNEL_14,
  ADC_CHANNEL_15,
  ADC_CHANNEL_TEMP,
  ADC_CHANNEL_REF,
  ADC_CHANNEL_BAT,
  NUM_ADC_CHANNELS,
} AdcChannel;

// Following typedef deprecated in favour of GpioAddress Version
typedef void (*AdcCallback)(AdcChannel adc_channel, void *context);

// Initialize the ADC to the desired conversion mode
void adc_init(AdcMode adc_mode);

// Enable or disable a given pin.
// A race condition may occur when setting a pin during a conversion.
// However, it should not cause issues given the intended use cases
// To set adc channels REF/TEMP/BAT, you must use adc_set_channel() below
StatusCode adc_set_channel_pin(GpioAddress address, bool new_state);

// Register a callback function to be called when the specified pin
// completes a conversion
StatusCode adc_register_callback_pin(GpioAddress address, AdcPinCallback callback, void *context);

// Obtain the raw 12-bit value read by the specified pin
StatusCode adc_read_raw_pin(GpioAddress address, uint16_t *reading);

// Obtain the converted value at the specified pin
StatusCode adc_read_converted_pin(GpioAddress address, uint16_t *reading);

// Following code and functions use adc channels as seen below
// but are now deprecated; they are still used in the current code base

// Enable or disable a given channel - must be used for channels TEMP/REF/BAT
// A race condition may occur when setting a channel during a conversion.
// However, it should not cause issues given the intended use cases
StatusCode adc_set_channel(AdcChannel adc_channel, bool new_state);

// Deprecated in favour of GpioAddress version
// Return a channel corresponding to the given GPIO address
StatusCode adc_get_channel(GpioAddress address, AdcChannel *adc_channel);

// Deprecated in favour of GpioAddress version
// Register a callback function to be called when the specified channel
// completes a conversion
StatusCode adc_register_callback(AdcChannel adc_channel, AdcCallback callback, void *context);

// Deprecated in favour of GpioAddress version
// Obtain the raw 12-bit value read by the specified channel
StatusCode adc_read_raw(AdcChannel adc_channel, uint16_t *reading);

// Deprecated in favour of GpioAddress version
// Obtain the converted value at the specified channel
StatusCode adc_read_converted(AdcChannel adc_channel, uint16_t *reading);
