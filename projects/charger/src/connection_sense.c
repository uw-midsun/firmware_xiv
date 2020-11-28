#include "connection_sense.h"

#include <stdint.h>

#include "adc.h"
#include "can_transmit.h"
#include "charger_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"

// refer to charger interface schematics for values
#define CS_UNPLUGGED_LOWER 2233
#define CS_UNPLUGGED_UPPER 2631
#define CS_PLUGGED_RELEASED_LOWER 754
#define CS_PLUGGED_RELEASED_UPPER 1116
#define CS_PLUGGED_PRESSED_LOWER 1459
#define CS_PLUGGED_PRESSED_UPPER 1938

static ConnectionState s_prev_state = CHARGER_STATE_UNPLUGGED;
static AdcChannel s_connection_adc_channel = NUM_ADC_CHANNELS;

static ConnectionState prv_read_conn_state() {
  uint16_t read = 0;
  adc_read_converted(s_connection_adc_channel, &read);
  if (CS_UNPLUGGED_LOWER < read && read < CS_UNPLUGGED_UPPER) {
    LOG_DEBUG("CHARGER_STATE_UNPLUGGED\n");
    return CHARGER_STATE_UNPLUGGED;
  } else if (CS_PLUGGED_RELEASED_LOWER < read && read < CS_PLUGGED_RELEASED_UPPER) {
    LOG_DEBUG("CHARGER_STATE_UNPLUGGED\n");
    return CHARGER_STATE_PLUGGED_RELEASED;
  } else if (CS_PLUGGED_PRESSED_LOWER < read && read < CS_PLUGGED_PRESSED_UPPER) {
    LOG_DEBUG("CHARGER_STATE_UNPLUGGED\n");
    return CHARGER_STATE_PLUGGED_PRESSED;
  } else {
    LOG_DEBUG("THIS SHOULDN'T HAPPEN\n");
    return NUM_CHARGER_CONNECTION_STATES;
  }
}

static void prv_periodic_cs_poll(SoftTimerId timer_id, void *context) {
  ConnectionState cur_state = prv_read_conn_state();
  if (cur_state == NUM_CHARGER_CONNECTION_STATES) {
    cur_state = CHARGER_STATE_UNPLUGGED;
  }

  if (cur_state != s_prev_state) {
    if (cur_state == CHARGER_STATE_UNPLUGGED) {
      CAN_TRANSMIT_CHARGER_CONNECTED_STATE(EE_CHARGER_CONN_STATE_DISCONNECTED);
      event_raise(CHARGER_CHARGE_EVENT_STOP, 0);
    } else if (cur_state == CHARGER_STATE_PLUGGED_RELEASED) {
      CAN_TRANSMIT_CHARGER_CONNECTED_STATE(EE_CHARGER_CONN_STATE_CONNECTED);
      event_raise(CHARGER_CHARGE_EVENT_BEGIN, 0);
    }
  }

  s_prev_state = cur_state;

  soft_timer_start_millis(CONNECTION_SENSE_POLL_PERIOD_MS, prv_periodic_cs_poll, NULL, NULL);
}

StatusCode connection_sense_init() {
  // Setup ADC to read
  GpioAddress cs_address = CONNECTION_SENSE_GPIO_ADDR;
  GpioSettings cs_settings = {
    .direction = GPIO_DIR_IN,          //
    .state = GPIO_STATE_LOW,           //
    .resistor = GPIO_RES_NONE,         //
    .alt_function = GPIO_ALTFN_ANALOG  //
  };

  gpio_init_pin(&cs_address, &cs_settings);
  adc_get_channel(cs_address, &s_connection_adc_channel);
  adc_set_channel(s_connection_adc_channel, true);

  s_prev_state = prv_read_conn_state();

  return soft_timer_start_millis(CONNECTION_SENSE_POLL_PERIOD_MS, prv_periodic_cs_poll, NULL, NULL);
}
