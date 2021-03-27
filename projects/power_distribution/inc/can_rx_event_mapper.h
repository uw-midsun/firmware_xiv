#pragma once

// Map CAN messages to events.
// Requires CAN and the event queue to be initialized.

#include "can.h"
#include "event_queue.h"

typedef struct {
  // The CAN message id we're responding to.
  CanMessageId msg_id;

  // Whether there is a "type" field in the first u16 slot.
  bool has_type;

  // These fields are used only if |has_type| is true.
  uint16_t *all_types;        // all types to which we'll respond; we ignore all others
  uint8_t num_types;          // length of preceding array
  EventId *type_to_event_id;  // map of the type field's value to the event ID raised

  // This field is used only if |has_type| is false.
  EventId event_id;  // the event ID to raise

  // Whether there is a "state" field. If |has_type| is true, we look for it in the second u16 slot;
  // else, we look for it in the first u16 slot. The state will be 1 if the state slot is nonzero
  // and 0 if it is zero.
  bool has_state;

  // Should we ack the message?
  bool ack;
} CanRxEventMapperMsgSpec;

typedef struct {
  CanRxEventMapperMsgSpec *msg_specs;
  uint8_t num_msg_specs;
} CanRxEventMapperConfig;

StatusCode power_distribution_can_rx_event_mapper_init(CanRxEventMapperConfig config);
