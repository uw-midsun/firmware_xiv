#pragma once
#include "ads1015.h"
#include "pedal_shared_resources_provider.h"

// The tentative values for the parking brake sensor are 3v3 for down and gnd for up

#define PARKING_BRAKE_SENSOR_DOWN_VOLTAGE 3.3
#define PARKING_BRAKE_SENSOR_UP_VOLTAGE 0

StatusCode get_brake_data(int16_t *position);
