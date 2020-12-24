// PCF8523 is a real-time clock (RTC) IC
#pragma once

#include <time.h>
#include "status.h"

typedef struct {
} Pcf8523Settings;

StatusCode pcf8523_init(Pcf8523Settings *settings);

tm pcf8523_get_time();

StatusCode pcf8523_set_time(tm *time);
