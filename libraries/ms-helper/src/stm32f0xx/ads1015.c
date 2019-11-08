#include "ads1015.h"
// The general idea is that the ALERT/RDY pin asserts whenever a conversion
// result is ready, and after storing the result the channel is switched to the
// next and ADS1015 is restarted for the new conversion. If no channels are
// enabled, the interrupt on ALERT/RDY pin is masked.
//
// Channel rotation is implemented through the use of bitsets. The main bitset
// holds the state of each channel(enable/disable). The pending bitset
// determines the next enabled channel by the find first set operation.
#include <status.h>
#include <string.h>
#include "ads1015_def.h"
#include "gpio_it.h"
#include "log.h"

#define ADS1015_DATA_RATE ADS1015_DATA_RATE_920

// Checks if a channel is enabled (true) or disabled (false).
static bool channel_is_enabled(Ads1015Storage *storage, Ads1015Channel channel) {
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

// Writes to register given upper and lower bytes.
static StatusCode prv_setup_register(Ads1015Storage *storage, uint8_t reg, uint8_t msb,
                                     uint8_t lsb) {
  uint8_t ads1015_setup_register[] = { reg, msb, lsb };
  return i2c_write(storage->i2c_port, storage->i2c_addr, ads1015_setup_register,
                   SIZEOF_ARRAY(ads1015_setup_register));
}

// Reads the register and stores the value in the given array.
static StatusCode prv_read_register(I2CPort i2c_port, uint8_t i2c_addr, uint8_t reg,
                                    uint8_t *rx_data, size_t rx_len) {
  status_ok_or_return(i2c_write(i2c_port, i2c_addr, &reg, sizeof(reg)));

  status_ok_or_return(i2c_read(i2c_port, i2c_addr, rx_data, rx_len));
  return STATUS_CODE_OK;
}

// Switches to the given channel by writing to config register.
static StatusCode prv_set_channel(Ads1015Storage *storage, Ads1015Channel channel) {
  if (channel >= NUM_ADS1015_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  status_ok_or_return(prv_setup_register(storage, ADS1015_ADDRESS_POINTER_CONFIG,
                                         ADS1015_CONFIG_REGISTER_MSB(channel),
                                         ADS1015_CONFIG_REGISTER_LSB(ADS1015_DATA_RATE)));
  storage->current_channel = channel;
  return STATUS_CODE_OK;
}

static void prv_watchdog(SoftTimerId timer_id, void *context) {
  Ads1015Storage *storage = context;

  if (!storage->watchdog_kicked) {
    // No interrupt when we should've gotten one by now
    // Mark data invalid and attempt to force a read
    storage->data_valid = false;
    gpio_it_trigger_interrupt(&storage->ready_pin);
  }
  storage->watchdog_kicked = false;

  soft_timer_start_millis(ADS1015_WATCHDOG_TIMEOUT_MS, prv_watchdog, storage,
                          &storage->watchdog_timer);
}

// This function is registered as the callback for ALRT/RDY Pin.
// Reads and stores the conversion value in storage, and switches to the next
// enabled channel. Also if there is a callback on a channel, it will be run
// here.
static void prv_interrupt_handler(const GpioAddress *address, void *context) {
  Ads1015Storage *storage = context;
  Ads1015Channel current_channel = storage->current_channel;
  uint8_t channel_bitset = storage->channel_bitset;
  uint8_t read_conv_register[2] = { 0, 0 };

  if (channel_is_enabled(storage, current_channel)) {
    prv_read_register(storage->i2c_port, storage->i2c_addr, ADS1015_ADDRESS_POINTER_CONV,
                      read_conv_register, SIZEOF_ARRAY(read_conv_register));
    // Following line puts the two read bytes into an int16.
    // 4 least significant bits are not part of the result hence the bitshift.
    storage->channel_readings[current_channel] =
        ((read_conv_register[0] << 8) | read_conv_register[1]) >>
        ADS1015_NUM_RESERVED_BITS_CONV_REG;

    storage->data_valid = true;

    // Runs the users callback if not NULL.
    if (storage->channel_callback[current_channel] != NULL) {
      storage->channel_callback[current_channel](current_channel,
                                                 storage->callback_context[current_channel]);
    }
  }

  prv_mark_channel_enabled(current_channel, false, &storage->pending_channel_bitset);
  if (storage->pending_channel_bitset == ADS1015_BITSET_EMPTY) {
    // Reset the pending bitset once gone through a cycle of channel rotation.
    storage->pending_channel_bitset = channel_bitset;
  }
  current_channel = __builtin_ffs(storage->pending_channel_bitset) - 1;
  // Update so that the ADS1015 reads from the next channel.
  prv_set_channel(storage, current_channel);

  storage->watchdog_kicked = true;
}

// Initiates ads1015 by setting up registers and enabling ALRT/RDY Pin.
// It also registers the interrupt handler on ALRT/RDY pin.
StatusCode ads1015_init(Ads1015Storage *storage, I2CPort i2c_port, Ads1015Address i2c_addr,
                        GpioAddress *ready_pin) {
  if ((storage == NULL) || (ready_pin == NULL)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(*storage));
  for (Ads1015Channel channel = 0; channel < NUM_ADS1015_CHANNELS; channel++) {
    storage->channel_readings[channel] = ADS1015_DISABLED_CHANNEL_READING;
  }
  storage->watchdog_timer = SOFT_TIMER_INVALID_TIMER;
  storage->watchdog_kicked = false;
  storage->data_valid = false;

  storage->i2c_port = i2c_port;
  storage->i2c_addr = i2c_addr + ADS1015_I2C_BASE_ADDRESS;
  storage->ready_pin = *ready_pin;
  // Set up config register.
  status_ok_or_return(prv_setup_register(storage, ADS1015_ADDRESS_POINTER_CONFIG,
                                         ADS1015_CONFIG_REGISTER_MSB_IDLE,
                                         ADS1015_CONFIG_REGISTER_LSB(ADS1015_DATA_RATE)));
  // Set up hi/lo-thresh registers. This particular setup enables the ALRT/RDY
  // pin.
  status_ok_or_return(prv_setup_register(storage, ADS1015_ADDRESS_POINTER_LO_THRESH,
                                         ADS1015_LO_THRESH_REGISTER_MSB,
                                         ADS1015_LO_THRESH_REGISTER_LSB));
  status_ok_or_return(prv_setup_register(storage, ADS1015_ADDRESS_POINTER_HI_THRESH,
                                         ADS1015_HI_THRESH_REGISTER_MSB,
                                         ADS1015_HI_THRESH_REGISTER_LSB));
  GpioSettings gpio_settings = {
    .direction = GPIO_DIR_IN,  //
  };
  InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };
  status_ok_or_return(gpio_init_pin(ready_pin, &gpio_settings));
  status_ok_or_return(gpio_it_register_interrupt(ready_pin, &it_settings, INTERRUPT_EDGE_RISING,
                                                 prv_interrupt_handler, storage));
  // Mask the interrupt until channels are enabled by the user.
  return gpio_it_mask_interrupt(ready_pin, true);
}

// This function enable/disables channels, and registers callbacks for each
// channel.
StatusCode ads1015_configure_channel(Ads1015Storage *storage, Ads1015Channel channel, bool enable,
                                     Ads1015Callback callback, void *context) {
  if (storage == NULL || channel >= NUM_ADS1015_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  status_ok_or_return(gpio_it_mask_interrupt(&storage->ready_pin, true));

  uint8_t channel_bitset = storage->channel_bitset;
  prv_mark_channel_enabled(channel, enable, &storage->channel_bitset);
  storage->pending_channel_bitset = storage->channel_bitset;
  storage->channel_callback[channel] = callback;
  storage->callback_context[channel] = context;

  if ((channel_bitset == ADS1015_BITSET_EMPTY) && enable) {
    // Set the given channel since the first channel is being enabled.
    status_ok_or_return(prv_set_channel(storage, channel));
  } else if (!enable) {
    // Set the reading to an invalid value if channel is being disabled.
    storage->channel_readings[channel] = ADS1015_DISABLED_CHANNEL_READING;
  }

  bool mask = (storage->channel_bitset == ADS1015_BITSET_EMPTY);
  // Unmask the interrupt if at least one channel is enabled.
  // Mask if all channels are disabled.
  status_ok_or_return(gpio_it_mask_interrupt(&storage->ready_pin, mask));

  if (!mask && storage->watchdog_timer == SOFT_TIMER_INVALID_TIMER) {
    storage->watchdog_kicked = false;
    soft_timer_start_millis(ADS1015_WATCHDOG_TIMEOUT_MS, prv_watchdog, storage,
                            &storage->watchdog_timer);
  } else if (mask) {
    soft_timer_cancel(storage->watchdog_timer);
  }
  return STATUS_CODE_OK;
}

// Reads raw 12 bit conversion results which are expressed in two's complement
// format.
StatusCode ads1015_read_raw(Ads1015Storage *storage, Ads1015Channel channel, int16_t *reading) {
  if (channel >= NUM_ADS1015_CHANNELS || storage == NULL || reading == NULL ||
      !channel_is_enabled(storage, channel)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (!storage->data_valid) {
    return status_code(STATUS_CODE_TIMEOUT);
  }

  *reading = storage->channel_readings[channel];
  return STATUS_CODE_OK;
}

// Reads conversion value in mVolt.
StatusCode ads1015_read_converted(Ads1015Storage *storage, Ads1015Channel channel,
                                  int16_t *reading) {
  if (channel >= NUM_ADS1015_CHANNELS || storage == NULL || reading == NULL ||
      !channel_is_enabled(storage, channel)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (!storage->data_valid) {
    return status_code(STATUS_CODE_TIMEOUT);
  }

  int16_t raw_reading = ADS1015_READ_UNSUCCESSFUL;
  status_ok_or_return(ads1015_read_raw(storage, channel, &raw_reading));
  *reading = (raw_reading * ADS1015_CURRENT_FSR) / ADS1015_NUMBER_OF_CODES;
  return STATUS_CODE_OK;
}
