#pragma once

// Retrieves pedal state from pedal_monitor.c and transmits CAN messages
// to control brake lights

// Requires pedal_monitor, CAN, GPIO, soft timers, event queue, and interrupts to be initialized.

#include "status.h"

#define PEDAL_STATE_UPDATE_FREQUENCY_MS 100

typedef enum {
  BRAKE_LIGHT_CONTROL_CAN_RX = 0,
  BRAKE_LIGHT_CONTROL_CAN_TX,
  BRAKE_LIGHT_CONTROL_FAULT,
  NUM_BRAKE_LIGHT_CONTROL_CAN_EVENTS
} BrakeLightControlEvent;

StatusCode brake_light_control_init(void);
