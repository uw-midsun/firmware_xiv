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

typedef enum {
  ADC_CHANNEL_TEMP = 16,
  ADC_CHANNEL_REF,
  ADC_CHANNEL_BAT,
  NUM_ADC_CHANNELS,
} AdcChannel;

typedef void (*AdcCallback)(GpioAddress address, void *context);

// Initialize the ADC to the desired conversion mode
void adc_init(AdcMode adc_mode);
// To access ADC_CHANNEL_TEMP, ADC_CHANNEL_REF, ADC_CHANNEL_BAT, initialize

// Enable or disable a given channel.
// A race condition may occur when setting a channel during a conversion.
// However, it should not cause issues given the intended use cases
StatusCode adc_set_channel(GpioAddress address, bool new_state);

// Register a callback function to be called when the specified channel
// completes a conversion
StatusCode adc_register_callback(GpioAddress address, AdcCallback callback, void *context);

// Obtain the raw 12-bit value read by the specified channel
StatusCode adc_read_raw(GpioAddress address, uint16_t *reading);

// Obtain the converted value at the specified channel
StatusCode adc_read_converted(GpioAddress address, uint16_t *reading);
