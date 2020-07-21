#pragma once

// Defines events used in solar.

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

// TODO(SOFT-215): Define more fault events
typedef enum {
  // An MCP3427 is faulting too much, event data is the DataPoint associated with the faulty MCP3427
  SOLAR_FAULT_EVENT_MCP3427 = NUM_SOLAR_DATA_EVENTS + 1,
  // An MPPT had an overcurrent, the least significant byte of event data is the index of the MPPT
  // that faulted, the most significant byte is a 4-bit bitmask of which branches faulted.
  SOLAR_FAULT_EVENT_MPPT_OVERCURRENT,
  // An MPPT had an overvoltage or overtemperature, event data is the index of the MPPT that faulted
  SOLAR_FAULT_EVENT_MPPT_OVERVOLTAGE,
  SOLAR_FAULT_EVENT_MPPT_OVERTEMPERATURE,
  NUM_SOLAR_FAULT_EVENTS,
} SolarFaultEvent;
