#pragma once
#include "ads1015.h"
#include "pedal_calib.h"

typedef struct PedalDataStorage {
  Ads1015Channel brake_channel;
  Ads1015Channel throttle_channel1;
  Ads1015Channel throttle_channel2;
} PedalDataStorage;

StatusCode pedal_data_init(Ads1015Storage *storage, PedalCalibBlob *calib_blob);

Ads1015Storage *get_ads1015_storage();

PedalDataStorage *get_pedal_data_storage();

PedalCalibBlob *get_pedal_calib_blob();
