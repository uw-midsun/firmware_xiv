#pragma once
#include "status.h"
#include "event_queue.h"
#include "charger_events.h"

#define CHARGER_BATTERY_VOLTAGE_THRESHOLD 999

StatusCode battery_voltage_monitor_init();
