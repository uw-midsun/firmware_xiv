#pragma once

// Handles the killswitch
// Requires requires gpio interrupts and soft timers to be initialized.

// Faults if the killswitch is hit as an input.

#include "status.h"

#define KS_MONITOR_PIN \
  { GPIO_PORT_A, 15 }

#define KS_ENABLE_PIN \
  { GPIO_PORT_B, 9 }

// Set the killswitch up to fault if hit. Killswitch is active-low.
StatusCode killswitch_init(void);
