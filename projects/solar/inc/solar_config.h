#pragma once

// Holds all hardware settings and configuration for solar.

#include "sense_current.h"

// after this many MCP3427 faults in a row, we raise a fault event
#define MAX_CONSECUTIVE_MCP3427_FAULTS 3

extern const SenseCurrentSettings sense_current_settings;
