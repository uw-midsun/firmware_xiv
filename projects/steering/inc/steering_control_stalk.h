#pragma once
// Module to set up analog readers to read voltages and convert
// them to signal events
#include "adc_periodic_reader.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "steering_can.h"

StatusCode control_stalk_init();
