#include <stdint.h>

#include "charger_connection_sense.h"
#include "charger_events.h"

#include "adc.h"
#include "event_queue.h"
#include "gpio.h"
#include "soft_timer.h"
#include "status.h"

static ChargerConnectionState s_prev_charger_state = CHARGER_STATE_UNPLUGGED;
static AdcChannel s_cs_adc_channel = NUM_ADC_CHANNELS;

static ChargerConnectionState prv_get_charger_state(AdcChannel channel) {
  uint16_t adc_read = 0;
  adc_read_converted(channel, &adc_read);
  if (CHARGER_CS_UNPLUGGED_LOWER < adc_read && adc_read < CHARGER_CS_UNPLUGGED_UPPER)
    return CHARGER_STATE_UNPLUGGED;
  else if (CHARGER_CS_PLUGGED_RELEASED_LOWER < adc_read &&
           adc_read < CHARGER_CS_PLUGGED_RELEASED_UPPER)
    return CHARGER_STATE_PLUGGED_RELEASED;
  else if (CHARGER_CS_PLUGGED_PRESSED_LOWER < adc_read &&
           adc_read < CHARGER_CS_PLUGGED_PRESSED_UPPER)
    return CHARGER_STATE_PLUGGED_PRESSED;
  else
    return NUM_CHARGER_CONNECTION_STATES;
}

static void prv_poll_connection_sense(SoftTimerId timer_id, void *context) {
  // Read the current state
  ChargerConnectionState cur_state = prv_get_charger_state(s_cs_adc_channel);
  // If it's the same as the state at the last read, do nothing
  if (cur_state == s_prev_charger_state) return;
  ChargerConnectionEvent event = NUM_CHARGER_CONNECTION_EVENTS;
  // If new state is unplugged, raise disconnected
  if (cur_state == CHARGER_STATE_UNPLUGGED) {
    event = CHARGER_DISCONNECTED_EVENT;
    // If new state is plugged, raise connected
  } else if (cur_state == CHARGER_STATE_PLUGGED_PRESSED ||
             cur_state == CHARGER_STATE_PLUGGED_RELEASED) {
    event = CHARGER_CONNECTED_EVENT;
  }
  event_raise(event, 0);
  s_prev_charger_state = cur_state;
  soft_timer_start_millis(CHARGER_CONNECTION_SENSE_POLL_RATE_MS, prv_poll_connection_sense, NULL,
                          NULL);
}

StatusCode connection_sense_init() {
  // Setup the ADC to read
  GpioAddress cs_addr = { .port = CHARGER_CONNECTION_SENSE_PORT,
                          .pin = CHARGER_CONNECTION_SENSE_PIN };
  GpioSettings cs_settings = { .direction = GPIO_DIR_IN,
                               .state = GPIO_STATE_LOW,
                               .resistor = GPIO_RES_NONE,
                               .alt_function = GPIO_ALTFN_ANALOG };
  gpio_init_pin(&cs_addr, &cs_settings);
  adc_init(ADC_MODE_SINGLE);
  adc_get_channel(cs_addr, &s_cs_adc_channel);
  adc_set_channel(s_cs_adc_channel, true);

  // Sets initial state to compare later reads against
  s_prev_charger_state = prv_get_charger_state(s_cs_adc_channel);

  return soft_timer_start_millis(CHARGER_CONNECTION_SENSE_POLL_RATE_MS, prv_poll_connection_sense,
                                 NULL, NULL);
}
