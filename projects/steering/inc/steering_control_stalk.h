#pragma once
// Module to set up analog readers to read voltages and convert
// them to signal events
#include "adc_periodic_reader.h"
#include "event_queue.h"
#include "status.h"

#define VOLTAGE_TOLERANCE_MV 100
#define STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV 1000
#define STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV 2000

StatusCode control_stalk_init();

void control_stalk_callback(uint16_t data, PeriodicReaderId id, void *context);
