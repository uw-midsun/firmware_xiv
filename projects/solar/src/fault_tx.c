#include "fault_tx.h"

#include <stdbool.h>

#include "can_transmit.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "solar_events.h"

bool fault_tx_process_event(Event *e) {
  if (e == NULL || e->id != SOLAR_FAULT_EVENT) return false;
  uint8_t fault = GET_FAULT_FROM_EVENT(*e);
  uint8_t data = GET_DATA_FROM_EVENT(*e);
  if (fault >= NUM_EE_SOLAR_FAULTS) return false;
  CAN_TRANSMIT_SOLAR_FAULT(fault, data);
  return true;
}
