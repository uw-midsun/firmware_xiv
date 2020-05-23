#include "adc.h"
#include "can.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define TIMER_TIMEOUT_IN_MILLIS 1000
#define AUX_VOLT_DEFAULT 0
#define AUX_TEMP_DEFAULT 0
#define AUX_OV 1 << 15
#define AUX_UV 1 << 14
#define AUX_OT 1 << 13
#define AUX_UT 1 << 12
#define DCDC_OFF 1 << 11

StatusCode aux_dcdc_monitor_init();
