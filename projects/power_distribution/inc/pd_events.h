#pragma once

#include "event_queue.h"

// publish_data transmits a lot of CAN messages, so it might overwhelm the EVENT_PRIORITY_NORMAL
// queue. To avoid losing important events, we transmit actionable events at a higher priority.
#define PD_ACTION_EVENT_PRIORITY EVENT_PRIORITY_HIGH
#define PD_BPS_STROBE_EVENT_PRIORITY EVENT_PRIORITY_HIGHEST

typedef enum {
  PD_CAN_EVENT_RX = 0,
  PD_CAN_EVENT_TX,
  PD_CAN_EVENT_FAULT,
  NUM_PD_CAN_EVENTS,
} PdCanEvent;

// Handled by pd_gpio
typedef enum {
  PD_GPIO_EVENT_DRIVER_DISPLAY = NUM_PD_CAN_EVENTS + 1,
  PD_GPIO_EVENT_STEERING,
  PD_GPIO_EVENT_CENTRE_CONSOLE,
  PD_GPIO_EVENT_DRL,
  PD_GPIO_EVENT_PEDAL,
  PD_GPIO_EVENT_HORN,
  PD_GPIO_EVENT_BRAKE_LIGHT,
  PD_GPIO_EVENT_STROBE,
  PD_GPIO_EVENT_SIGNAL_LEFT,
  PD_GPIO_EVENT_SIGNAL_RIGHT,
  PD_GPIO_EVENT_SIGNAL_HAZARD,
  NUM_PD_GPIO_EVENTS,
} PdGpioEvent;

// Handled by rear_strobe_blinker
typedef enum {
  PD_STROBE_EVENT = NUM_PD_GPIO_EVENTS + 1,
  NUM_PD_STROBE_EVENTS,
} PdStrobeEvent;

// Handled by a lights_signal_fsm instance
typedef enum {
  PD_SIGNAL_EVENT_LEFT = NUM_PD_STROBE_EVENTS + 1,
  PD_SIGNAL_EVENT_RIGHT,
  PD_SIGNAL_EVENT_HAZARD,
  NUM_PD_SIGNAL_EVENTS,
} PdSignalEvent;

// ALso handled by a lights_signal_fsm instance
typedef enum {
  PD_SYNC_EVENT_LIGHTS = NUM_PD_SIGNAL_EVENTS + 1,
  NUM_PD_SYNC_EVENTS,
} PdSyncEvent;

// Also handled by pd_gpio
typedef enum {
  PD_POWER_MAIN_SEQUENCE_EVENT_TURN_ON_DRIVER_DISPLAY_BMS = NUM_PD_SYNC_EVENTS + 1,
  PD_POWER_MAIN_SEQUENCE_EVENT_TURN_ON_EVERYTHING,
  PD_POWER_AUX_SEQUENCE_EVENT_TURN_ON_EVERYTHING,
  PD_POWER_OFF_SEQUENCE_EVENT_TURN_OFF_EVERYTHING,
  NUM_PD_POWER_SEQUENCE_EVENTS,
} PdPowerSequenceEvent;
