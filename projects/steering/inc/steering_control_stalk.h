#pragma once
// Module to set up analog readers to read voltages and convert
// them to signal events
#include "adc_periodic_reader.h"
#include "event_queue.h"
#include "status.h"

#define VOLTAGE_TOLERANCE_MV 20
#define STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV 3025
#define STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV 2984
#define DRL_SIGNAL_VOLTAGE_MV 1500
#define DRL_SIGNAL_VOLTAGE_TOLERANCE_MV 300

#define DRL_ON_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 6 }
#define DRL_OFF_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 5 }

StatusCode control_stalk_init();

void control_stalk_callback(uint16_t data, PeriodicReaderId id, void *context);
void drl_on_callback(uint16_t data, PeriodicReaderId id, void *context);
void drl_off_callback(uint16_t data, PeriodicReaderId id, void *context);
