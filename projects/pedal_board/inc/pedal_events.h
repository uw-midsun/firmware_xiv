#pragma once

typedef enum {
  PEDAL_CAN_RX = 0,
  PEDAL_CAN_TX,
  PEDAL_CAN_FAULT,
  NUM_PEDAL_CAN_EVENTS,
} PedalCanEvent;

typedef enum {
  PEDAL_PRESSED = 0,
  PEDAL_UNPRESSED,
  NUM_PEDAL_EVENTS,
} PedalState;
