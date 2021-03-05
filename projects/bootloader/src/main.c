#include "config.h"
#include "crc32.h"
#include "flash.h"
#include "interrupt.h"
#include "jump_to_application.h"
#include "log.h"
#include "soft_timer.h"

int main(void) {
  LOG_DEBUG("Hello from the bootloader!\n");
  // initialize all the modules
  flash_init();
  interrupt_init();
  soft_timer_init();
  crc32_init();
  config_init();

  jump_to_application();
  // not reached
  return 0;
}
