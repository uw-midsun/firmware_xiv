#include "drive_rx.h"

#include "can.h"
#include "can_transmit.h"
#include "can_unpack.h"

static void prv_handle_begin_precharge(const CanMessage* msg, void* context, CanAckStatus *ack_reply) {
    //begin precharge sequence
}

static void prv_handle_set_relay_states(const CanMessage* msg, void* context) {
    //go into neutral and open/close relays
}

// alters stored drive state
static void prv_handle_drive_state(const CanMessage* msg, void* context) {
    // update the fsm
}

// alters stored throttle value
static void prv_handle_throttle(const CanMessage* msg, void* context) {
    MotorControllerStorage* storage = context;
    uint16_t throttle_received = 0;
    CAN_UNPACK_THROTTLE_OUTPUT(msg, &throttle_received);
    storage->throttle = throttle_received;
}

// cancel cruise
static void prv_handle_brake(const CanMessage* msg, void* context) {
    //unimplemented
}

// returns an ack to powertrain heartbeat
static void prv_handle_heartbeat(const CanMessage* msg, CanAckStatus* ack_reply) {
    *ack_reply = CAN_ACK_STATUS_OK;
}

// initializes can rx handlers
StatusCode drive_rx_init(void* context) {
    can_register_rx_handler(SYSTEM_CAN_MESSAGE_BEGIN_PRECHARGE, prv_handle_begin_precharge, context);
    can_register_rx_handler(SYSTEM_CAN_MESSAGE_SET_RELAY_STATES, prv_handle_set_relay_states, context);
    can_register_rx_handler(SYSTEM_CAN_MESSAGE_CAR_DRIVE_STATE, prv_handle_drive_state, context);
    can_register_rx_handler(SYSTEM_CAN_MESSAGE_THROTTLE_OUTPUT, prv_handle_throttle, context);
    can_register_rx_handler(SYSTEM_CAN_MESSAGE_BRAKE_PRESSED, prv_handle_brake, context);
    can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT, prv_handle_heartbeat, context);
    return STATUS_CODE_OK;
}

