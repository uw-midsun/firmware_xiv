#pragma once
// Module for interacting with the ADS1015 ADC over I2C.
// I2C, GPIO, Interrupt, and Soft Timers should be initialized.
//
// The ADS1015 supports a conversion ready pin that we use as an interrupt.
//
// Uses a watchdog to detect if the ADS1015 has stopped triggering interrupts.
// Although we use GPIO interrupts to detect conversion ready, it seems like
// it's possible for us to miss it during bus glitching. This forces an
// interrupt if we haven't triggered within a few conversion periods.
#include <stdbool.h>
#include "gpio.h"
#include "i2c.h"
#include "soft_timer.h"
#include "status.h"

// Arbitrary watchdog timeout period
#define ADS1015_WATCHDOG_TIMEOUT_MS 100

typedef enum {
  ADS1015_ADDRESS_GND = 0,
  ADS1015_ADDRESS_VDD,
  ADS1015_ADDRESS_SDA,
  ADS1015_ADDRESS_SCL,
  NUM_ADS1015_ADDRESSES,
} Ads1015Address;

typedef enum {
  ADS1015_CHANNEL_0 = 0,
  ADS1015_CHANNEL_1,
  ADS1015_CHANNEL_2,
  ADS1015_CHANNEL_3,
  NUM_ADS1015_CHANNELS,
} Ads1015Channel;

// The callback runs after each conversion from the channel.
typedef void (*Ads1015Callback)(Ads1015Channel channel, void *context);

typedef struct Ads1015Storage {
  I2CPort i2c_port;
  uint8_t i2c_addr;
  GpioAddress ready_pin;
  int16_t channel_readings[NUM_ADS1015_CHANNELS];
  Ads1015Channel current_channel;
  uint8_t channel_bitset;
  uint8_t pending_channel_bitset;
  Ads1015Callback channel_callback[NUM_ADS1015_CHANNELS];
  void *callback_context[NUM_ADS1015_CHANNELS];

  SoftTimerId watchdog_timer;
  bool watchdog_kicked;
  bool data_valid;
} Ads1015Storage;

// Initiates ads1015 by setting up registers and enabling ALRT/RDY Pin.
StatusCode ads1015_init(Ads1015Storage *storage, I2CPort i2c_port, Ads1015Address i2c_addr,
                        GpioAddress *ready_pin);

// Enable/disables a channel, and registers a callback on the channel.
StatusCode ads1015_configure_channel(Ads1015Storage *storage, Ads1015Channel channel, bool enable,
                                     Ads1015Callback callback, void *context);

// Reads raw 12 bit conversion results which are expressed in two's complement
// format.
StatusCode ads1015_read_raw(Ads1015Storage *storage, Ads1015Channel channel, int16_t *reading);

// Reads conversion value in mVolt.
StatusCode ads1015_read_converted(Ads1015Storage *storage, Ads1015Channel channel,
                                  int16_t *reading);
