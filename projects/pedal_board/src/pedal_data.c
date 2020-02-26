#include "pedal_data.h"
#include "ads1015.h"
#include "log.h"

static Ads1015Storage *s_ads1015_storage;
static PedalDataStorage s_pedal_data_storage = { 
  .throttle_channel1 = ADS1015_CHANNEL_0,
  .throttle_channel2 = ADS1015_CHANNEL_1,
  .brake_channel = ADS1015_CHANNEL_2,
};

//perhaps also set the #define upper and lower here for calib
StatusCode pedal_data_init(Ads1015Storage *storage) {
  s_ads1015_storage = storage;

  // Throttle Channels, we only use 1 right now
  status_ok_or_return(ads1015_configure_channel(
      s_ads1015_storage, s_pedal_data_storage.throttle_channel1, true, NULL, NULL));
  status_ok_or_return(ads1015_configure_channel(
      s_ads1015_storage, s_pedal_data_storage.throttle_channel2, true, NULL, NULL));
  // brake channel
  status_ok_or_return(ads1015_configure_channel(
      s_ads1015_storage, s_pedal_data_storage.brake_channel, true, NULL, NULL));
  return STATUS_CODE_OK;
}

Ads1015Storage *get_ads1015_storage() {
  return s_ads1015_storage;
}

PedalDataStorage *get_pedal_data_storage() {
  return &s_pedal_data_storage;
}
