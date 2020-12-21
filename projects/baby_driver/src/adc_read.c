#include "adc_read.h"

#include "adc.h"
#include "babydriver_msg_defs.h"
#include "can_transmit.h"
#include "dispatcher.h"
#include "gpio.h"
#include "spi.h"

// Only first half of data is used. Data contains the id = 4 (BABYDRIVER_MESSAGE_ADC_READ_COMMAND
// id), port number, pin number and an int indicating if read should be raw or converted. Callback
// sends back a CAN message with id = 5 (BABYDRIVER_MESSAGE_ADC_READ_DATA), low byte, and high byte
// of ADC read conversion.
static StatusCode prv_adc_read_callback(uint8_t data[8], void *context, bool *tx_result) {
  uint16_t adc_pin_data = 0;
  bool is_raw;  // nonzero means raw, 0 means converted
  GpioAddress adc_pin_addr = { .port = data[1], .pin = data[2] };
  GpioSettings adc_pin_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  is_raw = (data[3] != 0);

  status_ok_or_return(gpio_init_pin(&adc_pin_addr, &adc_pin_settings));
  status_ok_or_return(adc_set_channel_pin(adc_pin_addr, true));

  if (is_raw) {
    status_ok_or_return(adc_read_raw_pin(adc_pin_addr, &adc_pin_data));
  } else {
    status_ok_or_return(adc_read_converted_pin(adc_pin_addr, &adc_pin_data));
  }

  uint8_t low = adc_pin_data & 0xff;
  uint8_t high = (adc_pin_data >> 8) & 0xff;

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_ADC_READ_DATA, low, high, 0, 0, 0, 0, 0);

  return STATUS_CODE_OK;
}

StatusCode adc_read_init(void) {
  return dispatcher_register_callback(BABYDRIVER_MESSAGE_ADC_READ_COMMAND, prv_adc_read_callback,
                                      NULL);
}
