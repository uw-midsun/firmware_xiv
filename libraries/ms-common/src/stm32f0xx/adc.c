#include "adc.h"

#include <stdbool.h>
#include <stddef.h>

#include "log.h"
#include "stm32f0xx.h"

// TS_CAL addresses obtained from section 3.10.1 of the specific device
// datasheet
#define ADC_TS_CAL1 0x1FFFF7b8
#define ADC_TS_CAL2 0x1FFFF7c2

// ADC_VREFINT_CAL address obtained from section 3.10.2 of the specific device
// datasheet
#define ADC_VREFINT_CAL 0x1FFFF7ba

typedef struct AdcStatus {
  uint32_t sequence;
  bool continuous;
  volatile bool converting;
} AdcStatus;

static AdcStatus s_adc_status;

// Functionalities used by GpioAddress version of adc library
typedef struct AdcInterrupt {
  AdcPinCallback pin_callback;
  AdcCallback callback;
  void *context;
  uint16_t reading;
} AdcInterrupt;

static AdcInterrupt s_adc_interrupts[NUM_ADC_CHANNELS];

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
  // To avoid dividing by zero faults:
  if (!reading) {
    return reading;
  }
  uint16_t vrefint_cal = *(uint16_t *)ADC_VREFINT_CAL;
  reading = (3300 * vrefint_cal) / reading;
  return reading;
}

static void prv_reset_channel(AdcChannel channel) {
  s_adc_interrupts[channel].callback = NULL;
  s_adc_interrupts[channel].pin_callback = NULL;
  s_adc_interrupts[channel].context = NULL;
  s_adc_interrupts[channel].reading = 0;
}

// used in IRQhandler to get pin from
static GpioAddress prv_channel_to_gpio(uint8_t adc_channel) {
  GpioAddress address;
  if (adc_channel >= 8 && adc_channel < 10) {
    address.port = GPIO_PORT_B;
    address.pin = adc_channel - 8;
  } else if (adc_channel >= 10 && adc_channel < ADC_CHANNEL_TEMP) {
    address.port = GPIO_PORT_C;
    address.pin = adc_channel - 10;
  } else {
    address.port = GPIO_PORT_A;
    address.pin = adc_channel;
  }
  return address;
}

static StatusCode prv_check_channel_valid_and_enabled(AdcChannel adc_channel) {
  if (adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (!(ADC1->CHSELR & ((uint32_t)1 << adc_channel))) {
    return status_code(STATUS_CODE_EMPTY);
  }
  return STATUS_CODE_OK;
}

void adc_init(AdcMode adc_mode) {
  ADC_DeInit(ADC1);

  // Once the ADC has been reset, enable it with the given settings
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, true);

  ADC_InitTypeDef adc_settings = {
    .ADC_Resolution = ADC_Resolution_12b,
    .ADC_ContinuousConvMode = adc_mode,
    .ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None,
    .ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_TRGO,
    .ADC_DataAlign = ADC_DataAlign_Right,
    .ADC_ScanDirection = ADC_ScanDirection_Upward,
  };

  ADC_Init(ADC1, &adc_settings);

  // Calculate the ADC calibration factor
  ADC_GetCalibrationFactor(ADC1);

  ADC_ContinuousModeCmd(ADC1, adc_mode);
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADCAL)) {
  }

  // Enable the ADC
  ADC_Cmd(ADC1, true);
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY)) {
  }

  ADC_WaitModeCmd(ADC1, true);
  ADC_AutoPowerOffCmd(ADC1, !adc_mode);

  // Enable interrupts for the end of each conversion
  stm32f0xx_interrupt_nvic_enable(ADC1_COMP_IRQn, INTERRUPT_PRIORITY_HIGH);
  ADC_ITConfig(ADC1, ADC_IER_EOCIE, true);
  ADC_ITConfig(ADC1, ADC_IER_EOSEQIE, true);

  // Initialize static variables
  s_adc_status.continuous = adc_mode;
  s_adc_status.sequence = 0;
  s_adc_status.converting = false;

  if (adc_mode) {
    ADC_StartOfConversion(ADC1);
  }

  for (size_t i = 0; i < NUM_ADC_CHANNELS; ++i) {
    prv_reset_channel(i);
  }

  // Configure internal reference channel to run by default for voltage
  // conversions
  adc_set_channel(ADC_CHANNEL_REF, true);
}

StatusCode adc_set_channel(AdcChannel adc_channel, bool new_state) {
  if (adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  if (new_state) {
    ADC_ChannelConfig(ADC1, ((uint32_t)1 << adc_channel), ADC_SampleTime_239_5Cycles);
  } else {
    ADC1->CHSELR &= ~((uint32_t)1 << adc_channel);
  }

  // Keep internal channels enabled only when set
  switch (adc_channel) {
    case ADC_CHANNEL_BAT:
      ADC_VbatCmd(new_state);
      break;

    case ADC_CHANNEL_REF:
      ADC_VrefintCmd(new_state);
      break;

    case ADC_CHANNEL_TEMP:
      ADC_TempSensorCmd(new_state);
      break;

    default:
      break;
  }

  s_adc_status.sequence = ADC1->CHSELR;
  return STATUS_CODE_OK;
}

// ADC Channel to GPIO Address mapping found in table 13 of the specific device
// datasheet. Channels 0 to 7 are occupied by port A, 8 to 9 by prt B, and 10 to
// 15 by port C
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
  status_ok_or_return(prv_check_channel_valid_and_enabled(adc_channel));
  prv_reset_channel(adc_channel);
  s_adc_interrupts[adc_channel].callback = callback;
  s_adc_interrupts[adc_channel].context = context;
  return STATUS_CODE_OK;
}

StatusCode adc_read_raw(AdcChannel adc_channel, uint16_t *reading) {
  status_ok_or_return(prv_check_channel_valid_and_enabled(adc_channel));

  if (!s_adc_status.continuous) {
    // SOFT-347: We previously waited while |ADC_GetFlagStatus(ADC1, ADC_FLAG_EOSEQ)| was true.
    // But the EOSEQ flag is normally 0 and is set to 1 at the end of conversion, but reset by
    // |ADC1_COMP_IRQHandler| before the interrupt finishes. So this loop always sees it as 0. Thus
    // we weren't waiting at all and just returning the old s_adc_interrupts[adc_channel].reading.
    // Fix: track whether we're converting with a volatile bool which is reset in the IRQHandler.
    // The ADC interrupt has INTERRUPT_PRIORITY_HIGH so it can interrupt soft timer callbacks and
    // other INTERRUPT_PRIORITY_NORMAL interrupts, but if this is called from another interrupt with
    // INTERRUPT_PRIORITY_HIGH, the NVIC will queue the ADC interrupt until the calling interrupt
    // returns and this loop will deadlock.
    s_adc_status.converting = true;
    ADC_StartOfConversion(ADC1);
    while (s_adc_status.converting) {
    }
  }

  *reading = s_adc_interrupts[adc_channel].reading;

  return STATUS_CODE_OK;
}

StatusCode adc_read_converted(AdcChannel adc_channel, uint16_t *reading) {
  status_ok_or_return(prv_check_channel_valid_and_enabled(adc_channel));
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

void ADC1_COMP_IRQHandler() {
  if (ADC_GetITStatus(ADC1, ADC_IT_EOC)) {
    uint16_t reading = ADC_GetConversionValue(ADC1);
    if (s_adc_status.sequence != 0) {
      AdcChannel current_channel = __builtin_ctz(s_adc_status.sequence);
      if (s_adc_interrupts[current_channel].callback != NULL) {
        s_adc_interrupts[current_channel].callback(current_channel,
                                                   s_adc_interrupts[current_channel].context);
      } else if (s_adc_interrupts[current_channel].pin_callback != NULL) {
        s_adc_interrupts[current_channel].pin_callback(prv_channel_to_gpio(current_channel),
                                                       s_adc_interrupts[current_channel].context);
      }
      s_adc_interrupts[current_channel].reading = reading;
      s_adc_status.sequence &= ~((uint32_t)1 << current_channel);
    }
  }

  if (ADC_GetITStatus(ADC1, ADC_IT_EOSEQ)) {
    s_adc_status.sequence = ADC1->CHSELR;
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOSEQ);
  }

  s_adc_status.converting = false;
}

// the following functions are wrappers over the legacy AdcChannel API dealing with GpioAddresses
// instead
StatusCode adc_set_channel_pin(GpioAddress address, bool new_state) {
  AdcChannel channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  return adc_set_channel(channel, new_state);
}

StatusCode adc_register_callback_pin(GpioAddress address, AdcPinCallback callback, void *context) {
  AdcChannel adc_channel;
  status_ok_or_return(adc_get_channel(address, &adc_channel));
  status_ok_or_return(prv_check_channel_valid_and_enabled(adc_channel));
  prv_reset_channel(adc_channel);
  s_adc_interrupts[adc_channel].pin_callback = callback;
  s_adc_interrupts[adc_channel].context = context;
  return STATUS_CODE_OK;
}

StatusCode adc_read_raw_pin(GpioAddress address, uint16_t *reading) {
  AdcChannel channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  return adc_read_raw(channel, reading);
}

StatusCode adc_read_converted_pin(GpioAddress address, uint16_t *reading) {
  AdcChannel channel;
  status_ok_or_return(adc_get_channel(address, &channel));
  return adc_read_converted(channel, reading);
}
