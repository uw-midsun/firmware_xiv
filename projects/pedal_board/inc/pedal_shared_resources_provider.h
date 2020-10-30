#pragma once
#include "ads1015.h"
#include "pedal_calib.h"

#define BRAKE_CHANNEL ADS1015_CHANNEL_1
#define THROTTLE_CHANNEL ADS1015_CHANNEL_0

StatusCode pedal_resources_init(Ads1015Storage *storage, PedalCalibBlob *calib_blob);

Ads1015Storage *get_shared_ads1015_storage();

PedalCalibBlob *get_shared_pedal_calib_blob();
