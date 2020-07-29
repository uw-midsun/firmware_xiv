#include "power_selection.h"
#include "adc.h"
#include "can.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "power_selection_events.h"
#include "resistance_to_temp.h"
#include "soft_timer.h"

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
// the first is voltage, the second is temp, last is current sense
AdcChannel aux_channels[AUX_ADC_CURRENT_CHANNEL] = { [AUX_ADC_VOLT_CHANNEL] = ADC_CHANNEL_0,
                                                     [AUX_ADC_TEMP_CHANNEL] = ADC_CHANNEL_1 };

static uint16_t s_status = 0;
static uint16_t s_aux_volt = 0;
static uint16_t s_aux_temp = 0;

// note: this section may be used as a smoke test, logging has been separated into this function
//The typical readings are:
//[0] projects/power_selection/src/power_selection.c:40: AUX Volatge Data: 2500
//[0] projects/power_selection/src/power_selection.c:41: AUX Temp Voltage Data: 2500
//[0] projects/power_selection/src/power_selection.c:42: AUX Temp Resistance Value: 3200.000000
//[0] projects/power_selection/src/power_selection.c:43: AUX Temp Data in C: 53

void smoke_test(uint16_t s_aux_volt, uint16_t s_aux_temp, double resistance, uint16_t s_aux_tempC) {
  LOG_DEBUG("AUX Volatge Data: %d\n", s_aux_volt);
  LOG_DEBUG("AUX Temp Voltage Data: %d\n", s_aux_temp);
  LOG_DEBUG("AUX Temp Resistance Value: %f\n", resistance);
  LOG_DEBUG("AUX Temp Data in C: %d\n", s_aux_tempC);
}

uint16_t prv_status_checker() {
  adc_read_raw(aux_channels[AUX_ADC_VOLT_CHANNEL], &s_aux_volt);
  adc_read_raw(aux_channels[AUX_ADC_TEMP_CHANNEL], &s_aux_temp);

  double resistance = temp_to_res(s_aux_temp);

  uint32_t s_aux_temp_prev = s_aux_temp;
  s_aux_temp = (uint16_t)resistance_to_temp(resistance);

  // calling the smoke test within this function as per the previous operation of power_selection.c
  smoke_test(s_aux_volt, s_aux_temp_prev, resistance, s_aux_temp);

  // see https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/1055326209/Power+Selector+Board for
  // the hardware specifications need to map data to voltages
  if ((s_aux_volt - AUX_VOLT_DEFAULT) < 9.1) {
    s_status = s_status & AUX_OV;
  } else if ((s_aux_volt) > 15) {
    s_status = s_status & AUX_UV;
  }
  // need to map data to temperatures (50 is 0 Celsius)
  if ((s_aux_temp - AUX_TEMP_DEFAULT) < 50) {
    s_status = s_status & AUX_OT;
  } else if ((s_aux_temp) > 95) {
    s_status = s_status & AUX_UT;
  }

  // checks if DCDC is on
  GpioState state = GPIO_STATE_HIGH;
  gpio_get_state(&s_dcdc_address, &state);
  if (state == GPIO_STATE_LOW) {
    s_status = s_status & DCDC_OFF;
  }

  return s_status;
}

static void prv_power_selection_callback(SoftTimerId timer_id, void *context) {
  uint16_t status = prv_status_checker();
  // SENDING AUX BATTERY DATA
  CAN_TRANSMIT_AUX_BATTERY_STATUS(s_aux_volt - AUX_VOLT_DEFAULT, s_aux_temp - AUX_TEMP_DEFAULT,
                                  status);
  soft_timer_start_millis(TIMER_TIMEOUT_MS, prv_power_selection_callback, context, NULL);
}

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  uint16_t status = prv_status_checker();
  uint16_t sequence = 0;
  CAN_UNPACK_POWER_ON_MAIN_SEQUENCE(msg, &sequence);

  if (sequence == EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS) {
    if ((status & AUX_STATUS) > 0) {
      *ack_reply = CAN_ACK_STATUS_INVALID;
    } else {
      *ack_reply = CAN_ACK_STATUS_OK;
    }
  }
  if (sequence == EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC) {
    if ((status & DCDC_OFF) > 0) {
      *ack_reply = CAN_ACK_STATUS_INVALID;
    } else {
      *ack_reply = CAN_ACK_STATUS_OK;
    }
  }

  return STATUS_CODE_OK;
}

StatusCode aux_dcdc_monitor_init() {
  for (int i = 0; i < AUX_ADC_CURRENT_CHANNEL; ++i) {
    gpio_init_pin(&s_aux_status_addresses[i], &s_gpio_settings);

    adc_get_channel(s_aux_status_addresses[i], &aux_channels[i]);
    adc_set_channel(aux_channels[i], true);
  }

  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE, prv_rx_callback, NULL));

  status_ok_or_return(
      soft_timer_start_millis(TIMER_TIMEOUT_MS, prv_power_selection_callback, NULL, NULL));
  return STATUS_CODE_OK;
}
