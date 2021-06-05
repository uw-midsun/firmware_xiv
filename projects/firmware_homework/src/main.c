#include "can_104.h"
#include "isc_104.h"
#include "spi_104.h"

int main() {
  // CAN command
  write_CAN_messages();

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
  }
  // I2C command
  send_i2c_message();
  // SPI command
  send_spi_message();

  return 0;
}
