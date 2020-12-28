#pragma once

#include <stdint.h>
// Binary coded decimal converter for two-digit numbers
// A value of 0 will be returned if an invalid value is passed in

uint8_t dec_to_bcd(uint8_t dec_value);

uint8_t bcd_to_dec(uint8_t bcd_value);
