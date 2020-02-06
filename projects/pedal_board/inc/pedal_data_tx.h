#pragma once
#include "ads1015.h"

typedef struct PedalData {
    Ads1015Storage *storage;
    Ads1015Channel brake_channel;
    Ads1015Channel throttle_channel;
} PedalData;

int16_t getBrakePosition();

int16_t getThrottlePosition();

// main should have a brake fsm, and ads1015storage
StatusCode pedal_data_tx_init(Ads1015Storage *storage);