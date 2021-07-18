#include "chip_id.h"

#include <stdint.h>

// Unique device identifier base address from section 33.1 of the specific device
// datasheet
#define DEVICE_ID_ADDR 0x1FFFF7AC

// returns chip id in the form of an array of uint32_ts
ChipId chip_id_get(void) {
  ChipId chip_id = { 0 };
  uint32_t *address_pointer = (uint32_t *)DEVICE_ID_ADDR;

  // Iterates through 96 bits of id and stores them in uint32_t array
  for (uint8_t i = 0; i < 2; i++) {
    chip_id.id[i] = *address_pointer;

    // Offsets address by 0x04
    address_pointer++;
  }

  return chip_id;
}
