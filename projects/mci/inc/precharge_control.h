#pragma once
#include "status.h"

// Requires GPIO to be initialized
// Requires GPIO interrupts to be initialized

typedef enum { MCI_PRECHARGE_DISCHARGED = 0, MCI_PRECHARGE_CHARGED } PrechargeState;

GpioState get_precharge_state(void *context);

StatusCode precharge_control_init(void *context);
