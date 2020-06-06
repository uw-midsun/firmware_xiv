#pragma once

// Defines events used in solar.

typedef enum {
  SOLAR_CAN_EVENT_RX = 0,
  SOLAR_CAN_EVENT_TX,
  SOLAR_CAN_EVENT_FAULT,
  NUM_SOLAR_CAN_EVENTS,
} SolarCanEvent;

typedef enum {
  DATA_READY_EVENT = NUM_SOLAR_CAN_EVENTS + 1,
  NUM_SOLAR_DATA_EVENTS,
} SolarDataEvent;

// TODO(SOFT-211,SOFT-215) Define fault events
