#include <string.h>
#include "centre_console_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "race_switch.h"
#include "unity.h"

static RaceSwitchFsmStorage s_race_switch_fsm_storage;

void setup_test(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  memset(&s_race_switch_fsm_storage, 0, sizeof(s_race_switch_fsm_storage));
  race_switch_fsm_init(&s_race_switch_fsm_storage);
}

void teardown_test(void) {}
