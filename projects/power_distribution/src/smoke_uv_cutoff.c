#include "can_transmit.h"
#include "front_uv_detector.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "pd_events.h"
#include "pin_defs.h"
#include "status.h"
#include "wait.h"

void smoke_uv_cutoff_perform(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  event_queue_init();

  // Initialize module for UV cutoff notification
  StatusCode front_uv_detector_init(GpioAddress * detector_pin);
  // initialize lights_signal_fsm
  SignalFsmSettings lights_signal_fsm_settings = {
    .signal_left_input_event = PD_SIGNAL_EVENT_LEFT,
    .signal_right_input_event = PD_SIGNAL_EVENT_RIGHT,
    .signal_hazard_input_event = PD_SIGNAL_EVENT_HAZARD,
    .signal_left_output_event = PD_GPIO_EVENT_SIGNAL_LEFT,
    .signal_right_output_event = PD_GPIO_EVENT_SIGNAL_RIGHT,
    .signal_hazard_output_event = PD_GPIO_EVENT_SIGNAL_HAZARD,
    .event_priority = PD_ACTION_EVENT_PRIORITY,
    .blink_interval_us = SIGNAL_BLINK_INTERVAL_US,
    .sync_behaviour = is_front_pd ? LIGHTS_SYNC_BEHAVIOUR_RECEIVE_SYNC_MSGS
                                  : LIGHTS_SYNC_BEHAVIOUR_SEND_SYNC_MSGS,
    .sync_event = PD_SYNC_EVENT_LIGHTS,
    .num_blinks_between_syncs = NUM_SIGNAL_BLINKS_BETWEEN_SYNCS,
  };
  BUG(lights_signal_fsm_init(&s_lights_signal_fsm_storage, &lights_signal_fsm_settings));
  static SignalFsmStorage s_lights_signal_fsm_storage;

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
      pd_gpio_process_event(&e);
      lights_signal_fsm_process_event(&s_lights_signal_fsm_storage, &e);
      if (!is_front_pd) {
        rear_strobe_blinker_process_event(&e);
      }
    }
    wait();
  }
}
