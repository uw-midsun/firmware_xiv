#pragma once
// Module for periodically sending CAN messages.
// Requires a generic can module, soft_timers and interrupts to be enabled.

#include <stdint.h>

#include "generic_can.h"
#include "generic_can_msg.h"
#include "soft_timer.h"
#include "status.h"

#define CAN_INTERVAL_POOL_SIZE 5

typedef struct CanInterval {
  GenericCan *can;
  GenericCanMsg msg;
  SoftTimerId timer_id;
  uint32_t period;
} CanInterval;

// Initializes the can interval module.
void can_interval_init(void);

// Returns |interval| a pointer to a CANInterval object which stores the
// provided settings. NOTE: msg will be persisted.
StatusCode can_interval_factory(const GenericCan *can, const GenericCanMsg *msg, uint32_t period,
                                CanInterval **interval);

// Immediately sends a message and continues to send messages. Requires the
// interval to be enabled.
StatusCode can_interval_send_now(CanInterval *interval);

// Sends a message periodically as specified by the settings in |interval|
// (should use can_interval_factory to initialize).
StatusCode can_interval_enable(CanInterval *interval);

// Stops sending a message periodically (|interval| should use
// can_interval_factory to initialize).
StatusCode can_interval_disable(CanInterval *interval);
