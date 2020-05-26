#pragma once
// Module to set up analog readers to read voltages and convert
// them to signal events
#include "adc_periodic_reader.h"
#include "event_queue.h"
#include "status.h"

#define VOLTAGE_TOLERANCE_MV 100
#define STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE 1000
#define STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE 2000
#define STEERING_CC_INCREASE_SPEED_VOLTAGE 3000
#define STEERING_CC_DECREASE_SPEED_VOLTAGE 4000
#define STEERING_CC_BRAKE_PRESSED_VOLTAGE 5000

StatusCode control_stalk_init();

// Used for testing by manually inserting values for voltage
void control_stalk_callback(uint16_t data, PeriodicReaderId id, void *context);
