#include "brake_state_monitor.h"
#include "can.h"

#define BRAKE_PRESSED_THRESHOLD 50

static BrakeState s_global_brake_state;

StatusCode brake_state_monitor_init(void) {
  return STATUS_CODE_OK;
}
