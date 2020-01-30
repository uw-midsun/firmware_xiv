#include "drive_rx.h"
#include "drive_fsm.h"
#include "motor_controller.h"

#include "can.h"
#include "can_unpack.h"
#include "can_ack.h"
#include "exported_enums.h"

// static StatusCode prv_handle_begin_precharge(const CanMessage* msg, void* context, CanAckStatus *ack_reply) {
//     //begin precharge sequence
//     return STATUS_CODE_OK;
// }

// static StatusCode prv_handle_set_relay_states(const CanMessage* msg, void* context , CanAckStatus *ack_reply) {
//     //go into neutral and open/close relays
//     return STATUS_CODE_OK;
// }

// alters stored drive state
static StatusCode prv_handle_drive_state(const CanMessage* msg, void* context, CanAckStatus *ack_reply) {
    uint8_t drive_state = 0;
    CAN_UNPACK_DRIVE_STATE(msg, &drive_state);
    switch (drive_state) {
        case EE_DRIVE_STATE_DRIVE:
            event_raise_priority(EVENT_PRIORITY_NORMAL, DRIVE_FSM_STATE_DRIVE, 0);
            break;
        case EE_DRIVE_STATE_REVERSE:
            event_raise_priority(EVENT_PRIORITY_NORMAL, DRIVE_FSM_STATE_REVERSE, 0);
            break;
        case EE_DRIVE_STATE_NEUTRAL:
        default:
            event_raise_priority(EVENT_PRIORITY_NORMAL, DRIVE_FSM_STATE_NEUTRAL, 0);
            break;
    }
    return STATUS_CODE_OK;
}

// alters stored throttle value
static StatusCode prv_handle_throttle(const CanMessage* msg, void* context, CanAckStatus *ack_reply) {
    MotorControllerStorage* storage = context;
    uint16_t throttle_received = 0;
    CAN_UNPACK_THROTTLE_OUTPUT(msg, &throttle_received);
    storage->throttle = (int16_t)throttle_received;
    return STATUS_CODE_OK;
}

// cancel cruise
// static StatusCode prv_handle_brake(const CanMessage* msg, void* context, CanAckStatus *ack_reply) {
//     //unimplemented
//     return STATUS_CODE_OK;
// }

// initializes can rx handlers
StatusCode drive_rx_init(void* context) {
    // can_register_rx_handler(SYSTEM_CAN_MESSAGE_BEGIN_PRECHARGE, prv_handle_begin_precharge, context);
    // can_register_rx_handler(SYSTEM_CAN_MESSAGE_SET_RELAY_STATES, prv_handle_set_relay_states, context);
    can_register_rx_handler(SYSTEM_CAN_MESSAGE_DRIVE_STATE, prv_handle_drive_state, context);
    can_register_rx_handler(SYSTEM_CAN_MESSAGE_THROTTLE_OUTPUT, prv_handle_throttle, context);
    // can_register_rx_handler(SYSTEM_CAN_MESSAGE_BRAKE_PRESSED, prv_handle_brake, context);
    return STATUS_CODE_OK;
}

