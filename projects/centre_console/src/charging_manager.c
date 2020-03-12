#include "main_event_generator.h"
#include "charging_manager.h"

ChargingState s_charging_state = NUM_CHARGING_STATES;

StatusCode init_charging_manager(void) {
  return STATUS_CODE_OK;
}

ChargingState get_charging_state(void) {
  return s_charging_state;
}
