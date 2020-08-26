#pragma once

#include "bms.h"
#include "status.h"

StatusCode fault_bps_init(BmsStorage *storage);

StatusCode fault_bps(uint8_t fault_bitmask, bool clear);
