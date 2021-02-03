#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#include "fw_104_can.h"
#include "fw_104_iic.h"
#include "fw_104_spi.h"

int main() {
  LOG_DEBUG("Program Started!\n");
  interrupt_init();
  event_queue_init();
  soft_timer_init();
  // prv_write_A_message();
  prv_write_B_message();

  while (true) {
    wait();
  }
  prv_write_read_spi_message();
  prv_write_read_i2c_message();

  return 0;
}
