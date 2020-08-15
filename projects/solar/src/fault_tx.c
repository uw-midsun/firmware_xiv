#include "fault_tx.h"

#include <stdbool.h>

#include "can_transmit.h"
#include "event_queue.h"
#include "solar_events.h"

bool fault_tx_process_event(Event *e) {
  if (e == NULL) return false;
  return true;
}
