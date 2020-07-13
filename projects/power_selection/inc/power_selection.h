#pragma once

#include "status.h"

#define TIMER_TIMEOUT_MS 1000
#define AUX_VOLT_DEFAULT 0
#define AUX_TEMP_DEFAULT -50
#define AUX_OV 1 << 15
#define AUX_UV 1 << 14
#define AUX_OT 1 << 13
#define AUX_UT 1 << 12
#define DCDC_OFF 1 << 11
#define AUX_STATUS 15 << 11

#define temp_to_res(r) 33000.0 / (double)((r) / 1000.0) - 10000

StatusCode aux_dcdc_monitor_init();
