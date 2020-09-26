#pragma once
// Module to set up analog readers to read voltages and convert
// them to signal events
#include "adc_periodic_reader.h"
#include "event_queue.h"
#include "status.h"

#define VOLTAGE_TOLERANCE_MV 10
#define STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV 2972
#define STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV 3013
#define STEERING_CONTROL_STALK_NO_SIGNAL_VOLTAGE_MV 3300

StatusCode control_stalk_init();

void control_stalk_callback(uint16_t data, PeriodicReaderId id, void *context);
