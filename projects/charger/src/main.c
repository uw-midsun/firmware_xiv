
// Charger Modules
// - charger_controller
//     - implements api for init, activate, and deactivate using mcp215
// - charger_controller_fault_monitor
//     - register can rx callbacks to find faults, then broadcasts them. implements init
// - [DONE] charger_control_pilot_monitor
//     - handles pwm reading requests, then raises an event with the result
//     - implements event_handler to be called from main
// - [DONE] charger_connection_sense
//     - handles polling connection sense pin, raises events to indicate state changes
//     - implements init
// - permission_resolver
//     - handles connection events
// - [TODOS] begin_charge_fsm
// - [TODOS] stop_charge_fsm
// - battery_voltage_monitor
//     - handles overvoltage faults from BMS

#include "begin_charge_fsm.h"
#include "charger_connection_sense.h"
#include "charger_control_pilot_monitor.h"
#include "stop_charger.h"

#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "soft_timer.h"
#include "status.h"

int main(void) {
  event_queue_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();

  connection_sense_init();
  control_pilot_monitor_init();
  begin_charge_fsm_init();

  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
      control_pilot_monitor_process_event(&e);
      begin_fsm_process_event(&e);
      stop_charger_process_event(&e);
    }
  }
  return 0;
}
