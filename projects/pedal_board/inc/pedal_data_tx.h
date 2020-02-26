#pragma once
#include "ads1015.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"

int16_t get_brake_position();

int16_t get_throttle_position();

StatusCode pedal_data_tx_init();
