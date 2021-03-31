#include "rear_strobe_blinker.h"
#include "blink_event_generator.h"
#include "pd_events.h"

static BlinkEventGeneratorStorage s_blinker_storage;

StatusCode rear_strobe_blinker_init(RearPowerDistributionStrobeBlinkerSettings *settings) {
  // Set up the blinker
  BlinkEventGeneratorSettings blinker_settings = {
    .interval_us = settings->strobe_blink_delay_us,
    .default_state = BLINKER_STATE_OFF,  // go back to off when we stop
    .callback = NULL,
  };
  return blink_event_generator_init(&s_blinker_storage, &blinker_settings);
}

StatusCode rear_strobe_blinker_process_event(Event *e) {
  if (e->id != POWER_DISTRIBUTION_STROBE_EVENT) return STATUS_CODE_OK;

  bool blinker_on = (e->data != 0);  // coalesce nonzero data to on
  if (blinker_on) {
    return blink_event_generator_start(&s_blinker_storage, POWER_DISTRIBUTION_GPIO_EVENT_STROBE);
  } else {
    blink_event_generator_stop(&s_blinker_storage);
    return STATUS_CODE_OK;
  }
}
