#pragma once
#include "ads1015.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"

typedef struct PedalDataStorage {
  Ads1015Storage *storage;
  Ads1015Channel brake_channel;
  Ads1015Channel throttle_channel1;
  Ads1015Channel throttle_channel2;
} PedalDataStorage;

int16_t get_brake_position();

int16_t get_throttle_position();

StatusCode pedal_data_tx_init(PedalDataStorage *storage);
