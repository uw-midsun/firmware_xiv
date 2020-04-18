#include "charging_manager.h"
#include "main_event_generator.h"

ChargingState s_charging_state = NUM_CHARGING_STATES;

StatusCode init_charging_manager(void) {
  s_charging_state = CHARGING_STATE_NOT_CHARGING;
  return STATUS_CODE_OK;
}

ChargingState *get_global_charging_state(void) {
  return &s_charging_state;
}
