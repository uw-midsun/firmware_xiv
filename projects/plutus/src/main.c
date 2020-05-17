#include <stddef.h>

#include "delay.h"
#include "log.h"
#include "plutus_cfg.h"

#include "can_transmit.h"
#include "fault_monitor.h"
#include "plutus_event.h"
#include "plutus_sys.h"
#include "wait.h"

static PlutusSysStorage s_plutus;
static FaultMonitorStorage s_fault_monitor;

static size_t s_telemetry_counter = 0;

int main(void) {
  PlutusSysType board_type = plutus_sys_get_type();
  plutus_sys_init(&s_plutus, board_type);
  LOG_DEBUG("Board type: %d\n", board_type);

  if (board_type == PLUTUS_SYS_TYPE_MASTER) {
    const FaultMonitorSettings fault_settings = {
      .ltc_afe = &s_plutus.ltc_afe,
      .current_sense = &s_plutus.current_sense,

      .overvoltage = PLUTUS_CFG_CELL_OVERVOLTAGE,
      .undervoltage = PLUTUS_CFG_CELL_UNDERVOLTAGE,

      .overcurrent_charge = PLUTUS_CFG_OVERCURRENT_DISCHARGE,
      .overcurrent_discharge = PLUTUS_CFG_OVERCURRENT_CHARGE,

      .overtemp_charge = PLUTUS_CFG_OVERTEMP_CHARGE,
      .overtemp_discharge = PLUTUS_CFG_OVERTEMP_DISCHARGE,
    };

    fault_monitor_init(&s_fault_monitor, &fault_settings);
  }

  current_sense_zero_reset(&s_plutus.current_sense);

  Event e = { 0 };
  while (true) {
    wait();
    while (status_ok(event_process(&e))) {
      if (board_type == PLUTUS_SYS_TYPE_MASTER) {
        fault_monitor_process_event(&s_fault_monitor, &e);
        ltc_afe_process_event(&s_plutus.ltc_afe, &e);
      }
    }
  }
}
