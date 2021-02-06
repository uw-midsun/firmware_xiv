#include "fw_104_can.h"
#include "fw_104_iic.h"
#include "fw_104_spi.h"

#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

int main() {
  LOG_DEBUG("Program Started!\n");
  interrupt_init();
  event_queue_init();
  soft_timer_init();

  // write_A_message();
  write_B_message();

  // The can_callback did not trigger unless I put this event queue stuff... (Copied from
  // smoke_can)
  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
  }

  write_read_spi_message();
  write_read_i2c_message();

  return 0;
}
