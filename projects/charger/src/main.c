
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
// - begin_charge_fsm
// - stop_charge_fsm
// - battery_voltage_monitor
//     - handles overvoltage faults from BMS

#include "charger_connection_sense.h"

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

  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
      // process events
    }
  }
  return 0;
}
