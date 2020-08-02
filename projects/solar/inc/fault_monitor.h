#pragma once

// Detects faults in data and raises fault events. Encapsulates all the fault conditions.
// Requires the event queue and the data store to be initialized.

#include "event_queue.h"
#include "solar_boards.h"
#include "status.h"

typedef struct FaultMonitorSettings {
  // Overcurrent threshold for the output current of the array. We also fault if it is negative.
  uint32_t output_overcurrent_threshold_uA;

  // Overvoltage threshold for the sum of the output voltages (sensed) from the MPPTs.
  // Only |mppt_count| voltages will be summed.
  uint32_t output_overvoltage_threshold_mV;

  // Overtemperature threshold for any individual thermistor.
  uint32_t overtemperature_threshold_dC;

  SolarMpptCount mppt_count;
} FaultMonitorSettings;

// Initialize the fault monitor with the given fault configuration.
StatusCode fault_monitor_init(FaultMonitorSettings *settings);

// Process an event, detect faults if it's a data ready event. Return whether it was processed.
bool fault_monitor_process_event(Event *event);
