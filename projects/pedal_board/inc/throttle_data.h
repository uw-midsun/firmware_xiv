#pragma once
#include "ads1015.h"
#include "pedal_data.h"

StatusCode get_throttle_data(PedalDataTxStorage *storage, int16_t *position);
