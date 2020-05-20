#pragma once
// Module to set up analog readers to read voltages and convert
// them to signal events
#include "adc_periodic_reader.h"
#include "event_queue.h"
#include "status.h"

StatusCode control_stalk_init();

// Used for testing by manually inserting values for voltage
void control_stalk_callback(uint16_t data, PeriodicReaderId id, void *context);
