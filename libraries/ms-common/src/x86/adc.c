#include <stddef.h>

#include "adc.h"
#include "interrupt.h"
#include "soft_timer.h"
// x86 implementation very similar to STM32F0 implementation.
// adc_read_raw should always return 4090.
// Vdda locked at 3300 mV.
// adc_read_converted should always return close to 2V

#define ADC_RETURNED_VOLTAGE_RAW 2500
#define ADC_CONTINUOUS_CB_FREQ_MS 50

// TS_CAL addresses obtained from section 3.10.1 of the specific device
// datasheet
#define ADC_TS_CAL1 0x1FFFF7b8
#define ADC_TS_CAL2 0x1FFFF7c2

// ADC_VREFINT_CAL address obtained from section 3.10.2 of the specific device
// datasheet
#define ADC_VREFINT_CAL 0x1FFFF7ba

typedef struct AdcInterrupt {
  AdcCallback callback;
  void *context;
  uint16_t reading;
} AdcInterrupt;

static AdcInterrupt s_adc_interrupts[NUM_ADC_CHANNELS];

static bool s_active_channels[NUM_ADC_CHANNELS];

// Formula obtained from section 13.9 of the reference manual. Returns reading
// in kelvin
static uint16_t prv_get_temp(uint16_t reading) {
  uint16_t ts_cal1 = *(uint16_t *)ADC_TS_CAL1;
  uint16_t ts_cal2 = *(uint16_t *)ADC_TS_CAL2;

  reading = ((110 - 30) * (reading - ts_cal1)) / (ts_cal2 - ts_cal1) + 30;

  return reading + 273;
}

// Formula obtained from section 13.9 of the reference manual. Returns Vdda in
// mV
static uint16_t prv_get_vdda(uint16_t reading) {
  return 3300;
}

static void prv_periodic_continous_cb(SoftTimerId id, void *context) {
  for (uint8_t i = 0; i < NUM_ADC_CHANNELS; i++) {
    if (s_adc_interrupts[i].callback != NULL) {
      s_adc_interrupts[i].callback(i, s_adc_interrupts[i].context);
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

  adc_set_channel(ADC_CHANNEL_REF, true);
}

StatusCode adc_set_channel(AdcChannel adc_channel, bool new_state) {
  if (adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_active_channels[adc_channel] = new_state;
  return STATUS_CODE_OK;
}

StatusCode adc_get_channel(GpioAddress address, AdcChannel *adc_channel) {
  *adc_channel = address.pin;

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

  if (*adc_channel > ADC_CHANNEL_15) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  return STATUS_CODE_OK;
}

StatusCode adc_register_callback(AdcChannel adc_channel, AdcCallback callback, void *context) {
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

StatusCode adc_read_raw(AdcChannel adc_channel, uint16_t *reading) {
  if (adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (s_active_channels[adc_channel] != true) {
    return status_code(STATUS_CODE_EMPTY);
  }

  // this section mimics the IRQ handler
  if (s_adc_interrupts[adc_channel].callback != NULL) {
    s_adc_interrupts[adc_channel].callback(adc_channel, s_adc_interrupts[adc_channel].context);
  }

  s_adc_interrupts[adc_channel].reading = ADC_RETURNED_VOLTAGE_RAW;
  *reading = s_adc_interrupts[adc_channel].reading;

  return STATUS_CODE_OK;
}

StatusCode adc_read_converted(AdcChannel adc_channel, uint16_t *reading) {
  if (adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (s_active_channels[adc_channel] != true) {
    return status_code(STATUS_CODE_EMPTY);
  }

  uint16_t adc_reading = 0;
  adc_read_raw(adc_channel, &adc_reading);

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
  adc_read_converted(ADC_CHANNEL_REF, &vdda);
  *reading = (adc_reading * vdda) / 4095;

  return STATUS_CODE_OK;
}
