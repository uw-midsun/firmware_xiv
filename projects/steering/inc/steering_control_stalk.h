#pragma once
// Module to set up analog readers to read voltages and convert
// them to signal events
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "steering_can.h"

#define STEERING_CONTROL_STALK_LEFT_VOLTAGE=1000;
#define STEERING_CONTROL_STALK_LEFT_VOLTAGE=2000;
#define STEERING_CC_INCREASE_SPEED_VOLTAGE=2000;
#define STEERING_CC_INCREASE_SPEED_VOLTAGE=2500;
#define STEERING_CC_DECREASE_SPEED_VOLTAGE=2500;
#define STEERING_CC_BRAKE_PRESSED_VOLTAGE=3000;

StatusCode control_stalk_init();

//For brake pressed events
StatusCode control_stalk_process_event();