#pragma once

// Defines events used in solar.

#include "event_queue.h"

#define DATA_READY_EVENT_PRIORITY EVENT_PRIORITY_NORMAL
#define FAULT_EVENT_PRIORITY EVENT_PRIORITY_HIGH

typedef enum {
  SOLAR_CAN_EVENT_RX = 0,
  SOLAR_CAN_EVENT_TX,
  SOLAR_CAN_EVENT_FAULT,
  NUM_SOLAR_CAN_EVENTS,
} SolarCanEvent;

typedef enum {
  SOLAR_MCP3427_EVENT_DATA_TRIGGER = NUM_SOLAR_CAN_EVENTS + 1,
  SOLAR_MCP3427_EVENT_DATA_READY,
  NUM_SOLAR_MCP3427_EVENTS
} SolarMcp3427Event;

typedef enum {
  DATA_READY_EVENT = NUM_SOLAR_MCP3427_EVENTS + 1,
  NUM_SOLAR_DATA_EVENTS,
} SolarDataEvent;

// The specific faults are from EESolarFault in the exported enums.
// Use the macros below for handling fault events.
typedef enum {
  SOLAR_FAULT_EVENT = NUM_SOLAR_DATA_EVENTS + 1,
  NUM_SOLAR_FAULT_EVENTS,
} SolarFaultEvent;

#define RAISE_FAULT_EVENT(fault, data) \
  event_raise_priority(FAULT_EVENT_PRIORITY, SOLAR_FAULT_EVENT, FAULT_EVENT_DATA(fault, data))
#define FAULT_EVENT_DATA(fault, data) (((fault) << 8) | (data))
#define GET_FAULT_FROM_EVENT(e) ((e).data >> 8)
#define GET_DATA_FROM_EVENT(e) ((e).data & 0xFF)

// Used by solar_fsm internally.
typedef enum {
  SOLAR_COMMAND_EVENT_CLOSE_RELAY = NUM_SOLAR_FAULT_EVENTS + 1,
  SOLAR_COMMAND_EVENT_OPEN_RELAY,
  NUM_SOLAR_EXTERNAL_COMMAND_EVENTS,
} SolarExternalCommandEvent;
