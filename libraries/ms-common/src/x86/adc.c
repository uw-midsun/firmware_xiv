#include "adc.h"

void adc_init(AdcMode adc_mode) {}

StatusCode adc_set_channel(AdcChannel adc_channel, bool new_state) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode adc_get_channel(GpioAddress address, AdcChannel *adc_channel) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode adc_register_callback(AdcChannel adc_channel, AdcCallback callback, void *context) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode adc_read_raw(AdcChannel adc_channel, uint16_t *reading) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode adc_read_converted(AdcChannel adc_channel, uint16_t *reading) {
  return STATUS_CODE_UNIMPLEMENTED;
}
