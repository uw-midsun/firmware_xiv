#pragma once

#include "gpio.h"
#include "status.h"

// Checks if UV cutoff has occurred on front PD and sends CAN message to telemetry.
// This should only be called in front power distribution.
// Requires GPIO, GPIO interrupts, and interrupts to be initialized.

// Initialize module for UV cutoff notification
StatusCode front_uv_detector_init(GpioAddress *detector_pin);
