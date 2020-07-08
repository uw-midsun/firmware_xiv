#pragma once

#include "power_selection.h"
#include "adc.h"

void log_analog_pins(void);

AdcChannel aux_channels2[AUX_ADC_CURRENT_CHANNEL] = { [AUX_ADC_VOLT_CHANNEL] = ADC_CHANNEL_0,
                                                     [AUX_ADC_TEMP_CHANNEL] = ADC_CHANNEL_1,
                                                     };
void log_analog_pins(void) {
  static uint16_t s_status = 0;
  static uint16_t s_aux_volt = 0;
  static uint16_t s_aux_temp = 0;
  static uint16_t s_aux_curr = 0;
  adc_read_raw(aux_channels2[AUX_ADC_VOLT_CHANNEL], &s_aux_volt);
  adc_read_raw(aux_channels2[AUX_ADC_TEMP_CHANNEL], &s_aux_temp);
  //adc_read_raw(aux_channels[AUX_ADC_CURRENT_CHANNEL], &s_aux_curr); //AUX_ADC_CURR_CHANNEL
  LOG_DEBUG("AUX Voltage Data: %d\n", s_aux_volt);
  LOG_DEBUG("AUX Temp Voltage Data: %d\n", s_aux_temp);
  //LOG_DEBUG("AUX Curr Voltage Data: %d\n", s_aux_curr);
}