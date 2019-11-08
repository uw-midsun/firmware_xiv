#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "can_msg.h"
#include "status.h"

typedef struct GenericCanMsg {
  uint32_t id;
  uint64_t data;
  size_t dlc;
  bool extended;
} GenericCanMsg;

// Converts a GenericCanMsg to a CanMessage.
StatusCode generic_can_msg_to_can_message(const GenericCanMsg *src, CanMessage *dst);

// Converts a CanMessage to a GenericCanMsg.
StatusCode can_message_to_generic_can_message(const CanMessage *src, GenericCanMsg *dst);
