#include "chip_id.h"

#include <sys/types.h>
#include <unistd.h>

// returns process id in the form of an array of uint16_ts
ChipId chip_id_get(void) {
  ChipId chip_id = { 0 };
  pid_t process_id;
  process_id = getpid();

  // Max of uint32_t is larger than max of pid_t
  chip_id.id[0] = (uint32_t)process_id;

  return chip_id;
}
