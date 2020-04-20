#include "adc.h"
#include "can.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define TIMER_TIMEOUT_IN_MILLIS 1000

// change later
static GpioAddress s_dcdc_address = { .port = GPIO_PORT_A, .pin = 1 };
static GpioAddress s_aux_address = { .port = GPIO_PORT_A, .pin = 2 };
static GpioSettings s_gpio_settings = {
  GPIO_DIR_IN,        //
  GPIO_STATE_LOW,     //
  GPIO_RES_NONE,      //
  GPIO_ALTFN_ANALOG,  //
};
AdcChannel aux_channel = NUM_ADC_CHANNELS;
AdcChannel dcdc_channel = NUM_ADC_CHANNELS;

static void prv_power_selection_callback(SoftTimerId timer_id, void *context) {
  // SENDING AUX BATTERY DATA
  // CAN_TRANSMIT_AUX_BAT_DATA()
  soft_timer_start_millis(TIMER_TIMEOUT_IN_MILLIS, prv_power_selection_callback, context, NULL);
}

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  LOG_DEBUG("Received a message!\n");

  if (msg->data_u16 == EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS) {
  }
  if (msg->data_u16 == EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC) {
  }

  return STATUS_CODE_OK;
}

static void checker(AdcChannel channel) {
    uint16_t data = 0;
    adc_read_raw(channel, &data);
    LOG_DEBUG("potentiometer data: %d\n", data);
    
}

static void aux_dcdc_monitor_init() {
  gpio_init_pin(&s_aux_address, &s_gpio_settings);
  gpio_init_pin(&s_dcdc_address, &s_gpio_settings);

  adc_get_channel(s_aux_address, &aux_channel);
  adc_set_channel(aux_channel, true);

  adc_get_channel(s_dcdc_address, &dcdc_channel);
  adc_set_channel(dcdc_channel, true);

  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE, prv_rx_callback, NULL));

  status_ok_or_return(
      soft_timer_start_millis(TIMER_TIMEOUT_IN_MILLIS, prv_power_selection_callback, NULL, NULL));
  return STATUS_CODE_OK;
}
