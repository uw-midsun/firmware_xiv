#include "permission_resolver.h"
#include "charger_events.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"

StatusCode prv_permission_rx(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
    // don't need any conditional logic because conditionals handled by center console
    event_raise(BEGIN_CHARGE_FSM_EVENT_PENDING_STATE, 0);
}

bool permission_resolver_process_event(const Event *e) {
    if (e->id == CHARGER_CONNECTION_EVENT_CONNECTED) {
        // send can message to center console
        CAN_TRANSMIT_REQUEST_TO_CHARGE();
    }
}

StatusCode permission_resolver_init() {
    // register can rx
    can_register_rx_handler(SYSTEM_CAN_MESSAGE_ALLOW_CHARGING, prv_permission_rx, NULL);
    return STATUS_CODE_OK;
}