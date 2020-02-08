#pragma once
#include "status.h"

// Requires GPIO to be initialized
// Requires GPIO interrupts to be initialized

typedef enum { MCI_PRECHARGE_DISCHARGED = 0, MCI_PRECHARGE_CHARGED } PrechargeState;

StatusCode precharge_control_init(void *context);
