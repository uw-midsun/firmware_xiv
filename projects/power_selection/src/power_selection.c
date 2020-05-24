#include "power_selection.h"

#include "adc.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "power_selection_events.h"
#include "soft_timer.h"

// not needed in the module currently
static GpioAddress s_dcdc_status_addresses[3] = {
  { .port = GPIO_PORT_A, .pin = 1 },  // dcdc voltage sense
  { .port = GPIO_PORT_A, .pin = 4 },  // dcdc temp sense
  { .port = GPIO_PORT_A, .pin = 6 }   // dcdc current sense
};

// dcdc address if on. Pull Down is means it's on
static GpioAddress s_dcdc_address = { .port = GPIO_PORT_A, .pin = 9 };
static GpioAddress s_aux_status_addresses[3] = {
  { .port = GPIO_PORT_A, .pin = 0 },  // aux voltage sense
  { .port = GPIO_PORT_A, .pin = 3 },  // aux temp sense
  { .port = GPIO_PORT_A, .pin = 5 }   // aux current sense  //not needed in the module currently
};

static GpioSettings s_gpio_settings = {
  GPIO_DIR_IN,        //
  GPIO_STATE_LOW,     //
  GPIO_RES_NONE,      //
  GPIO_ALTFN_ANALOG,  //
};
// the first is voltage the second is temp
AdcChannel aux_channels[3] = { NUM_ADC_CHANNELS, NUM_ADC_CHANNELS, NUM_ADC_CHANNELS };
// again not used currently
AdcChannel dcdc_channels[3] = { NUM_ADC_CHANNELS, NUM_ADC_CHANNELS, NUM_ADC_CHANNELS };

static uint16_t prv_checker() {
  uint16_t aux_volt = 0;
  uint16_t aux_temp = 0;
  uint16_t status = 0;

  adc_read_raw(aux_channels[0], &aux_volt);
  adc_read_raw(aux_channels[1], &aux_temp);
  // LOG_DEBUG("AUX Volatge Data: %d\n", aux_volt);
  // LOG_DEBUG("AUX Temp Data: %d\n", aux_temp);

  // need to map data to voltages
  if ((aux_volt - AUX_VOLT_DEFAULT) < 9.1) {
    status = status & AUX_OV;
  } else if ((aux_volt) > 15) {
    status = status & AUX_UV;
  }
  // need to map data to temperatures (50 is 0 Celsius)
  if ((aux_temp - AUX_TEMP_DEFAULT) < 50) {
    status = status & AUX_OT;
  } else if ((aux_temp) > 95) {
    status = status & AUX_UT;
  }

  // checks if DCDC is on
  GpioState state = GPIO_STATE_HIGH;
  gpio_get_state(&s_dcdc_address, &state);
  if (state == GPIO_STATE_LOW) {
    status = status & DCDC_OFF;
  }

  return status;
}

static void prv_power_selection_callback(SoftTimerId timer_id, void *context) {
  uint16_t aux_volt = 0;
  uint16_t aux_temp = 0;
  uint16_t status = prv_checker();

  adc_read_raw(aux_channels[0], &aux_volt);
  adc_read_raw(aux_channels[1], &aux_temp);
  // SENDING AUX BATTERY DATA
  CAN_TRANSMIT_AUX_BATTERY_STATUS(aux_volt - AUX_VOLT_DEFAULT, aux_temp - AUX_TEMP_DEFAULT, status);
  soft_timer_start_millis(TIMER_TIMEOUT_IN_MILLIS, prv_power_selection_callback, context, NULL);
}

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  uint16_t status = prv_checker();

  if (msg->data_u16[0] == EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS) {
    if ((status >> 11) > 0) {
      *ack_reply = CAN_ACK_STATUS_INVALID;
    } else {
      *ack_reply = CAN_ACK_STATUS_OK;
    }
  }
  if (msg->data_u16[0] == EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC) {
    if ((status << 4) > 0) {
      *ack_reply = CAN_ACK_STATUS_INVALID;
    } else {
      *ack_reply = CAN_ACK_STATUS_OK;
    }
  }

  return STATUS_CODE_OK;
}

StatusCode aux_dcdc_monitor_init() {
  for (int i = 0; i < 3; ++i) {
    gpio_init_pin(&s_aux_status_addresses[i], &s_gpio_settings);
    // gpio_init_pin(&s_dcdc_status_addresses[i], &s_gpio_settings);

    adc_get_channel(s_aux_status_addresses[i], &aux_channels[i]);
    adc_set_channel(aux_channels[i], true);

    // adc_get_channel(s_dcdc_status_addresses[i], &dcdc_channels[i]);
    // adc_set_channel(dcdc_channels[i], true);
  }

  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE, prv_rx_callback, NULL));

  status_ok_or_return(
      soft_timer_start_millis(TIMER_TIMEOUT_IN_MILLIS, prv_power_selection_callback, NULL, NULL));
  return STATUS_CODE_OK;
}
