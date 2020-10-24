#pragma once

#include "bms.h"
#include "status.h"

StatusCode fault_bps_init(BmsStorage *storage);

StatusCode fault_bps_set(uint8_t fault_bitmask);

StatusCode fault_bps_clear(uint8_t fault_bitmask);
