#include "bcd.h"

uint8_t dec_to_bcd(uint8_t dec_value) {
  if (dec_value > 99) {
    return 0XFF;
  }
  uint8_t tens = dec_value / 10;
  uint8_t ones = dec_value % 10;
  return (tens << 4) + ones;
}

uint8_t bcd_to_dec(uint8_t bcd_value) {
  uint8_t tens = (bcd_value & 0b11110000) >> 4;
  uint8_t ones = bcd_value & 0b00001111;
  return (tens > 9 || ones > 9) ? 0XFF : (tens * 10 + ones);
}
