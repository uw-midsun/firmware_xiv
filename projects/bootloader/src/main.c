#include "jump_to_application.h"
#include "log.h"

int main(void) {
  LOG_DEBUG("Hello from the bootloader!\n");
  jump_to_application();
  // not reached
  return 0;
}
