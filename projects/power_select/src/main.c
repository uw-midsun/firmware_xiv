// Power select FW implementation

#include "can.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "generic_can.h"
#include "interrupt.h"
#include "log.h"
#include "power_select.h"
#include "power_select_defs.h"
#include "power_select_events.h"
#include "power_select_can.h"



int main() {
  interrupt_init();
  gpio_init();
  soft_timer_init();
  adc_init(ADC_MODE_SINGLE);

  event_queue_init();

  LOG_DEBUG("init: %d\n", power_select_init());
  LOG_DEBUG("can init: %d\n", power_select_can_init());
  power_select_start();

  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    can_process_event(&e);
  }
  return 0;
}