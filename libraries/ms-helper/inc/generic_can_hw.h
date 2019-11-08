#pragma once
// Wrapper around CAN Hw for use of extended ids with Generic CAN.
// This performs the initialization of can_hw.
//
// NOTE: this is primarily intended for testing. It isn't nearly as fault
// tolerant as Network Layer CAN and should only be used in situations where
// faults are permissible and message traffic is light (i.e. communicating with
// a device not on the primary CAN Network).

#include "can_hw.h"
#include "event_queue.h"
#include "generic_can.h"
#include "status.h"

typedef struct GenericCanHw {
  GenericCan base;
  EventId fault_event;
} GenericCanHw;

StatusCode generic_can_hw_init(GenericCanHw *can_hw, const CanHwSettings *settings,
                               EventId fault_event);
