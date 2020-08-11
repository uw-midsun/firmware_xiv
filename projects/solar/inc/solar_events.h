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

typedef enum {
  // An MCP3427 is faulting too much, event data is the DataPoint associated with the faulty MCP3427
  SOLAR_FAULT_EVENT_MCP3427 = NUM_SOLAR_DATA_EVENTS + 1,

  // An MPPT had an overcurrent, the least significant byte of event data is the index of the MPPT
  // that faulted, the most significant byte is a 4-bit bitmask of which branches faulted.
  SOLAR_FAULT_EVENT_MPPT_OVERCURRENT,

  // An MPPT had an overvoltage or overtemperature, event data is the index of the MPPT that faulted
  SOLAR_FAULT_EVENT_MPPT_OVERVOLTAGE,
  SOLAR_FAULT_EVENT_MPPT_OVERTEMPERATURE,

  // The current from the whole array is over the threshold. No event data.
  SOLAR_FAULT_EVENT_OVERCURRENT,

  // The current from the whole array is negative, so we aren't charging. No event data.
  SOLAR_FAULT_EVENT_NEGATIVE_CURRENT,

  // The sum of the sensed voltages is over the threshold. No event data.
  SOLAR_FAULT_EVENT_OVERVOLTAGE,

  // The temperature of any array thermistor is over our threshold. Event data is the index of the
  // too-hot thermistor.
  SOLAR_FAULT_EVENT_OVERTEMPERATURE,

  NUM_SOLAR_FAULT_EVENTS,
} SolarFaultEvent;

typedef enum {
  SOLAR_COMMAND_EVENT_CLOSE_RELAY = NUM_SOLAR_FAULT_EVENTS + 1,
  SOLAR_COMMAND_EVENT_OPEN_RELAY,
  NUM_SOLAR_EXTERNAL_COMMAND_EVENTS,
} SolarExternalCommandEvent;
