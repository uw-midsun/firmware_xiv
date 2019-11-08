// This module emulates the behavior of ADS1015 on x86.
// Instead of having interrupts raised by the device once a channel finishes
// conversion, there is a soft timer with prv_timer_callback that is called
// periodically to update channel readings and switch to the next channel. The
// rotation of the channels is implemented using bitsets. The readings are just
// a arbitrary value.
#include "ads1015.h"
#include <string.h>
#include "ads1015_def.h"
#include "soft_timer.h"

#define ADS1015_CHANNEL_UPDATE_PERIOD_US ADS1015_CONVERSION_TIME_US_1600_SPS
#define ADS1015_CHANNEL_ARBITRARY_READING 0

// Checks if a channel is enabled (true) or disabled (false).
static bool prv_channel_is_enabled(Ads1015Storage *storage, Ads1015Channel channel) {
  return ((storage->channel_bitset & (1 << channel)) != 0);
}

// Updates the channel_bitset when a channel is enabled/disabled.
static void prv_mark_channel_enabled(Ads1015Channel channel, bool enable, uint8_t *channel_bitset) {
  if (enable) {
    *channel_bitset |= (1 << channel);
  } else {
    *channel_bitset &= ~(1 << channel);
  }
}

// Sets the current channel of the storage.
static StatusCode prv_set_channel(Ads1015Storage *storage, Ads1015Channel channel) {
  if (channel >= NUM_ADS1015_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  storage->current_channel = channel;
  return STATUS_CODE_OK;
}

// Periodically calls channels' callbacks imitating the interrupt behavior.
static void prv_timer_callback(SoftTimerId id, void *context) {
  Ads1015Storage *storage = context;
  Ads1015Channel current_channel = storage->current_channel;

  if (prv_channel_is_enabled(storage, current_channel)) {
    storage->channel_readings[current_channel] = ADS1015_CHANNEL_ARBITRARY_READING;
    // Runs the users callback if not NULL.
    if (storage->channel_callback[current_channel] != NULL) {
      storage->channel_callback[current_channel](current_channel,
                                                 storage->callback_context[current_channel]);
    }
  }
  // Disable the channel on the pending bitset.
  prv_mark_channel_enabled(current_channel, false, &storage->pending_channel_bitset);
  // Reset the pending bitset once gone through a cycle of channel rotation.
  if (storage->pending_channel_bitset == ADS1015_BITSET_EMPTY) {
    storage->pending_channel_bitset = storage->channel_bitset;
  }
  // Obtain the next enabled channel.
  current_channel = (Ads1015Channel)(__builtin_ffs(storage->pending_channel_bitset) - 1);
  // Update so that the ADS1015 reads from the next channel.
  prv_set_channel(storage, current_channel);
  soft_timer_start(ADS1015_CHANNEL_UPDATE_PERIOD_US, prv_timer_callback, storage, NULL);
}

// Inits the storage for ADS1015 and starts the soft timer.
StatusCode ads1015_init(Ads1015Storage *storage, I2CPort i2c_port, Ads1015Address i2c_addr,
                        GpioAddress *ready_pin) {
  if (storage == NULL || ready_pin == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(Ads1015Storage));
  for (Ads1015Channel channel = 0; channel < NUM_ADS1015_CHANNELS; channel++) {
    storage->channel_readings[channel] = ADS1015_DISABLED_CHANNEL_READING;
  }
  return soft_timer_start(ADS1015_CHANNEL_UPDATE_PERIOD_US, prv_timer_callback, storage, NULL);
}

// Enable/disables a channel, and sets a callback for the channel.
StatusCode ads1015_configure_channel(Ads1015Storage *storage, Ads1015Channel channel, bool enable,
                                     Ads1015Callback callback, void *context) {
  if (storage == NULL || channel >= NUM_ADS1015_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  prv_mark_channel_enabled(channel, enable, &storage->channel_bitset);
  storage->pending_channel_bitset = storage->channel_bitset;
  storage->channel_callback[channel] = callback;
  storage->callback_context[channel] = context;

  if (!enable) {
    storage->channel_readings[channel] = ADS1015_DISABLED_CHANNEL_READING;
  }
  return STATUS_CODE_OK;
}

// Reads raw results from the storage.
StatusCode ads1015_read_raw(Ads1015Storage *storage, Ads1015Channel channel, int16_t *reading) {
  if (channel >= NUM_ADS1015_CHANNELS || storage == NULL || reading == NULL ||
      !prv_channel_is_enabled(storage, channel)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *reading = storage->channel_readings[channel];
  return STATUS_CODE_OK;
}

// Reads conversion value in mVolt.
StatusCode ads1015_read_converted(Ads1015Storage *storage, Ads1015Channel channel,
                                  int16_t *reading) {
  if (channel >= NUM_ADS1015_CHANNELS || storage == NULL || reading == NULL ||
      !prv_channel_is_enabled(storage, channel)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  int16_t raw_reading = ADS1015_READ_UNSUCCESSFUL;
  status_ok_or_return(ads1015_read_raw(storage, channel, &raw_reading));
  *reading = (raw_reading * ADS1015_CURRENT_FSR) / ADS1015_NUMBER_OF_CODES;
  return STATUS_CODE_OK;
}
