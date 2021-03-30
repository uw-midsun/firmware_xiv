#include "fault_handler.h"

#include <stdint.h>

#include "can_transmit.h"
#include "exported_enums.h"
#include "log.h"
#include "relay_fsm.h"
#include "status.h"

static EESolarFault s_relay_open_faults[MAX_RELAY_OPEN_FAULTS];
static size_t s_num_relay_open_faults = 0;
static SolarMpptCount s_mppt_count;

StatusCode fault_handler_init(FaultHandlerSettings *settings) {
  if (settings == NULL || settings->num_relay_open_faults > MAX_RELAY_OPEN_FAULTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  for (size_t i = 0; i < settings->num_relay_open_faults; i++) {
    s_relay_open_faults[i] = settings->relay_open_faults[i];
  }
  s_num_relay_open_faults = settings->num_relay_open_faults;
  s_mppt_count = settings->mppt_count;

  return STATUS_CODE_OK;
}

StatusCode fault_handler_raise_fault(EESolarFault fault, uint8_t fault_data) {
  if (fault >= NUM_EE_SOLAR_FAULTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  LOG_WARN("Fault raised! Fault ID = %d, data = 0x%x\n", fault, fault_data);

  StatusCode status = STATUS_CODE_OK;
  for (size_t i = 0; i < s_num_relay_open_faults; i++) {
    if (fault == s_relay_open_faults[i]) {
      StatusCode relay_open_status = relay_fsm_open();
      if (!status_ok(relay_open_status)) status = relay_open_status;
      break;
    }
  }

<<<<<<< HEAD
  StatusCode can_status = CAN_TRANSMIT_SOLAR_FAULT_5_MPPTS(fault, fault_data);
=======
  StatusCode can_status = STATUS_CODE_UNINITIALIZED;
  if (s_mppt_count == SOLAR_BOARD_5_MPPTS) {
    can_status = CAN_TRANSMIT_SOLAR_FAULT_5_MPPTS(fault, fault_data);
  } else if (s_mppt_count == SOLAR_BOARD_6_MPPTS) {
    can_status = CAN_TRANSMIT_SOLAR_FAULT_6_MPPTS(fault, fault_data);
  }
>>>>>>> master
  if (!status_ok(can_status)) status = can_status;

  return status;
}
