#include "charging_manager.h"

#include "can_transmit.h"
#include "can_unpack.h"
#include "drive_fsm.h"
#include "exported_enums.h"
#include "log.h"
#include "main_event_generator.h"

ChargingState s_charging_state = NUM_CHARGING_STATES;

StatusCode prv_disconnect_rx_handler(const CanMessage *msg, void *context, CanAckStatus *ack) {
  EEChargerConnState conn_state = NUM_EE_CHARGER_CONN_STATES;
  CAN_UNPACK_CHARGER_CONNECTED_STATE(msg, (uint8_t *)&conn_state);
  if (conn_state == EE_CHARGER_CONN_STATE_DISCONNECTED) {
    s_charging_state = CHARGING_STATE_NOT_CHARGING;
  }
  return STATUS_CODE_OK;
}

StatusCode prv_permission_rx_handler(const CanMessage *msg, void *context, CanAckStatus *ack) {
  DriveState *drive_state = context;
  // Charging should only be allowed if we're parked.
  if (*drive_state == DRIVE_STATE_PARKING) {
    CAN_TRANSMIT_ALLOW_CHARGING();
    s_charging_state = CHARGING_STATE_CHARGING;
  }
  // if we're in the wrong state, do nothing. Unplug/replug charger to try again.
  return STATUS_CODE_OK;
}

StatusCode init_charging_manager(const DriveState *drive_state) {
  s_charging_state = CHARGING_STATE_NOT_CHARGING;
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_REQUEST_TO_CHARGE, prv_permission_rx_handler,
                          drive_state);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_CHARGER_CONNECTED_STATE, prv_disconnect_rx_handler,
                          NULL);
  return STATUS_CODE_OK;
}

ChargingState get_global_charging_state(void) {
  return s_charging_state;
}
