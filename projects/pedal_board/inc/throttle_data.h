#pragma once
#include "ads1015.h"

StatusCode getThrottleData(Ads1015Storage *storage, Ads1015Channel channel, int16_t *position);
