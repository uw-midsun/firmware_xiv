#pragma once
// Module for interfacing to CAN for CAN controlled relays.
//
// Requires CAN to be initialized.
//
// Allows registering a RelayRxHandler that is wrapped by a CanRxHandlerCb which
// is properly configured to unpack relay messages and handle faults. This
// RelayRxHandler is expected to execute the modifications to any GPIO pins or
// other mechanisms to trigger state changes to relays and report them back to
// the CANRxHanderCb which will handle the ack. These should ideally be kept
// very short, just GPIO manipulation or enqueueing events!

#include <stdint.h>

#include "can_msg_defs.h"
#include "status.h"

// Wrapped by a CanRxHandlerCb.
typedef StatusCode (*RelayRxHandler)(SystemCanMessage msg_id, uint8_t state, void *context);

typedef struct RelayRxStorage {
  RelayRxHandler handler;
  SystemCanMessage msg_id;  // Unused but helpful in debugging.
  uint8_t curr_state;
  uint8_t state_bound;  // Non-inclusive upper bound |[0, state_bound)| on
                        // |curr_state|.
  void *context;
} RelayRxStorage;

// Configures |RelayRxHandler| to be triggered when |msg_id| is received. This
// handler should alter a relay or set of relays to match the expected state. In
// the event of a failure the status code should propagate back to the
// CanRxHandler. |state_bound| is the non-inclusive upper bound on the values
// the returned uint8_t can be. The configuration is stored in |storage|. If the
// relay is already in the expected state |handler| should still return with
// STATUS_CODE_OK so long as there are no faults.
//
// NOTE: we explicitly don't constrain |msg_id|. In theory we could force this
// to be a value defined for one of the relays in codegen-tooling. But this
// module could be used for any CAN controlled element that uses a message
// format of a single uint8_t. Also note that the |storage| is also extensible
// for this reason.
StatusCode relay_rx_configure_handler(RelayRxStorage *storage, SystemCanMessage msg_id,
                                      uint8_t state_bound, RelayRxHandler handler, void *context);
