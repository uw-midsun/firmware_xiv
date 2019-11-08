#pragma once
// CAN TX/RX event handlers
// This is an internal module and should not be used.

// The idea is that the CAN RX ISR pushes received message into a queue and
// raises an RX event. In the FSM, when we encounter an RX event, we pop the
// messages in the RX queue and decide what to do based on the event type:
// * Data: Find registered RX handler and run the corresponding callback. If an
// ACK was requested,
//         we send an ACK as a response. The callback may choose to set the
//         ACK's status.
// * ACK: Find the corresponding pending ACK request and process the ACK.

// For TX, its primary purpose is rate-limiting. The idea is that we raise TX
// events to process requested transmits in the main loop instead of chaining
// them off of the TX ready interrupt. This prevents transmits from starving the
// main loop.

// We expect TX and RX events to be 1-to-1 and that discarded events will be
// re-raised externally.
#include "can.h"
#include "fsm.h"

StatusCode can_fsm_init(Fsm *fsm, CanStorage *can_storage);
