#pragma once
#include "ads1015.h"

typedef struct BrakeData {
    Ads1015Storage *storage;
    uint16_t *position;
} BrakeData;

StatusCode brake_data_init(Ads1015Storage *storage, uint16_t *brake_position);
