// PM flow should be something like
  // 1. [DONE] build project
  // 2. [DONE] run project and return PID
  // 3. block on driver init
  // 4. harness shm_open O_CREAT using PID in name
  // 5. harness mmap
  // 6. driver shm_open
  // 7. driver mmap
  // 8. driver continues init
// Protos required assuming we start with GPIO
  // message GpioPort - 16 pins (bool), 16 interrupt IDs (uint8_t)
  // message GpioState - 6 ports
// Protos must go in libraries/mpxe-gen, to be used by mpxe and projects

// Must implement worker threads to pull from module rings to main ring

#include <stdio.h>

#include "pm.h"

int main(void) {
  build_project("can_communication");
  run_project("can_communication");
  return 0;
}
