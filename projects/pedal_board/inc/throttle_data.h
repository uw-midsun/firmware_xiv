#pragma once
#include "ads1015.h"
#include "pedal_data_tx.h"

StatusCode get_throttle_data(PedalDataStorage *storage, int16_t *position);
