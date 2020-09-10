#include <stddef.h>

#include "adc.h"
#include "interrupt.h"
#include "soft_timer.h"
// x86 implementation very similar to STM32F0 implementation.
// adc_read_raw should always return 4090.
// Vdda locked at 3300 mV.
// adc_read_converted should always return close to 2V
// temperature reading always returns 293 kelvin.

#define ADC_RETURNED_VOLTAGE_RAW 2500
#define ADC_CONTINUOUS_CB_FREQ_MS 50
#define ADC_TEMP_RETURN 293
#define ADC_VDDA_RETURN 3300

typedef struct AdcInterrupt {
  AdcCallback callback;
  void *context;
  uint16_t reading;
} AdcInterrupt;

static AdcInterrupt s_adc_interrupts[NUM_ADC_CHANNELS];

static bool s_active_channels[NUM_ADC_CHANNELS];

static uint16_t prv_get_temp(uint16_t reading) {
  return ADC_TEMP_RETURN;
}

static uint16_t prv_get_vdda(uint16_t reading) {
  return ADC_VDDA_RETURN;
}

// ADC Channel to GPIO Address mapping found in table 13 of the specific device
// datasheet. Channels 0 to 7 are occupied by port A, 8 to 9 by prt B, and 10 to
// 15 by port C
static StatusCode prv_adc_get_channel(GpioAddress address, uint8_t *adc_channel) {
  *adc_channel = address.pin;
  if (address.pin < ADC_CHANNEL_TEMP) {
    switch (address.port) {
      case GPIO_PORT_A:
        if (address.pin > 7) {
          return status_code(STATUS_CODE_INVALID_ARGS);
        }
        break;
      case GPIO_PORT_B:
        if (address.pin > 1) {
          return status_code(STATUS_CODE_INVALID_ARGS);
        }
        *adc_channel += 8;
        break;
      case GPIO_PORT_C:
        if (address.pin > 5) {
          return status_code(STATUS_CODE_INVALID_ARGS);
        }
        *adc_channel += 10;
        break;
    }
  }
  if (*adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  return STATUS_CODE_OK;
}

static void prv_periodic_continous_cb(SoftTimerId id, void *context) {
  GpioAddress address;
  for (uint8_t i = 0; i < NUM_ADC_CHANNELS; i++) {
    if (s_adc_interrupts[i].callback != NULL) {
      address.pin = i;
      s_adc_interrupts[i].callback(address, s_adc_interrupts[i].context);
    }
  }
  soft_timer_start_millis(ADC_CONTINUOUS_CB_FREQ_MS, prv_periodic_continous_cb, NULL, NULL);
}

void adc_init(AdcMode adc_mode) {
  interrupt_init();
  soft_timer_init();
  if (adc_mode == ADC_MODE_CONTINUOUS) {
    soft_timer_start_millis(ADC_CONTINUOUS_CB_FREQ_MS, prv_periodic_continous_cb, NULL, NULL);
  }
  GpioAddress ref_address = { .pin = ADC_CHANNEL_REF };
  adc_set_channel(ref_address, true);
}

StatusCode adc_set_channel(GpioAddress address, bool new_state) {
  uint8_t adc_channel;
  prv_adc_get_channel(address, &adc_channel);
  if (adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_active_channels[adc_channel] = new_state;
  return STATUS_CODE_OK;
}

StatusCode adc_register_callback(GpioAddress address, AdcCallback callback, void *context) {
  uint8_t adc_channel;
  prv_adc_get_channel(address, &adc_channel);
  if (adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (s_active_channels[adc_channel] != true) {
    return status_code(STATUS_CODE_EMPTY);
  }

  s_adc_interrupts[adc_channel].callback = callback;
  s_adc_interrupts[adc_channel].context = context;

  return STATUS_CODE_OK;
}

StatusCode adc_read_raw(GpioAddress address, uint16_t *reading) {
  uint8_t adc_channel;
  prv_adc_get_channel(address, &adc_channel);
  if (adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (s_active_channels[adc_channel] != true) {
    return status_code(STATUS_CODE_EMPTY);
  }

  s_adc_interrupts[adc_channel].reading = ADC_RETURNED_VOLTAGE_RAW;
  *reading = s_adc_interrupts[adc_channel].reading;

  // this section mimics the IRQ handler
  if (s_adc_interrupts[adc_channel].callback != NULL) {
    s_adc_interrupts[adc_channel].callback(address, s_adc_interrupts[adc_channel].context);
  }

  return STATUS_CODE_OK;
}

StatusCode adc_read_converted(GpioAddress address, uint16_t *reading) {
  uint8_t adc_channel;
  prv_adc_get_channel(address, &adc_channel);
  if (adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (s_active_channels[adc_channel] != true) {
    return status_code(STATUS_CODE_EMPTY);
  }

  uint16_t adc_reading = 0;
  adc_read_raw(address, &adc_reading);
  switch (adc_channel) {
    case ADC_CHANNEL_TEMP:
      *reading = prv_get_temp(adc_reading);
      return STATUS_CODE_OK;

    case ADC_CHANNEL_REF:
      *reading = prv_get_vdda(adc_reading);
      return STATUS_CODE_OK;

    case ADC_CHANNEL_BAT:
      adc_reading *= 2;
      break;

    default:
      break;
  }

  uint16_t vdda;
  GpioAddress ref_address = {
    .pin = ADC_CHANNEL_REF,
  };
  adc_read_converted(ref_address, &vdda);
  *reading = (adc_reading * vdda) / 4095;

  return STATUS_CODE_OK;
}
