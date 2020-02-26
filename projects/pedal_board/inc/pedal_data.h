#pragma once
#include "ads1015.h"

typedef struct PedalDataStorage {
  Ads1015Channel brake_channel;
  Ads1015Channel throttle_channel1;
  Ads1015Channel throttle_channel2;
} PedalDataStorage;

StatusCode pedal_data_init(Ads1015Storage *storage);

Ads1015Storage *get_ads1015_storage();

PedalDataStorage *get_pedal_data_storage();
