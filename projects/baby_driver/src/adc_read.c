#include "adc_read.h"

#include <stdint.h>
#include <string.h>

#include "adc.h"
#include "babydriver_msg_defs.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "dispatcher.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static GpioAddress s_adc_pin_addr;
static AdcChannel s_adc_pin_channel = NUM_ADC_CHANNELS;

static GpioSettings s_adc_pin_settings = {
  GPIO_DIR_IN,
  GPIO_STATE_LOW,
  GPIO_RES_NONE,
  GPIO_ALTFN_ANALOG,
};

static void prv_adc_read_callback(BabydriverMessageId id, uint8_t data[8],
                                  const GpioAddress *address, void *context) {
  uint16_t adc_pin_data = 0;

  bool tx_result = true;
  bool is_raw;  // nonzero means raw, 0 means converted

  s_adc_pin_addr.port = data[1];
  s_adc_pin_addr.pin = data[2];
  is_raw = data[3];

  gpio_init_pin(&s_adc_pin_addr, &s_adc_pin_settings);

  if (is_raw) {
    adc_read_raw_pin(s_adc_pin_addr, adc_pin_data);
  } else {
    adc_read_converted_pin(s_adc_pin_addr, &adc_pin_data);
  }

  uint8_t low = adc_pin_data & 0xff;
  uint8_t high = (adc_pin_data >> 8) & 0xff;

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_ADC_READ_DATA, low, high, 0, 0, 0, 0, 0);
  return STATUS_CODE_OK;
}

StatusCode adc_read_init(void) {
  dispatcher_register_callback(BABYDRIVER_MESSAGE_ADC_READ_COMMAND, prv_adc_read_callback, NULL);
}
