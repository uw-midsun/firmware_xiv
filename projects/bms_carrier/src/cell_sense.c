#include "cell_sense.h"

#include "status_code.h"

static CellSenseStorage s_storage = { 0 };

StatusCode cell_sense_init(LtcAfeStorage *afe, CellSenseSettings *settings) {  
  s_storage->afe = afe;
  s_storage->overvoltage = settings->overvoltage;
  s_storage->undervoltage = settings->undervoltage;
  s_storage->overtemp_charge = settings->overtemp_charge;
  s_storage->overtemp_discharge = settings->overtemp_discharge;
  // TODO(SOFT-9): Add soft-timer for kicking off readings
  // TODO(SOFT-9): Add event handler for reading completion
  return STATUS_CODE_OK;
}
