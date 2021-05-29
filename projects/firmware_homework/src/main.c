#include "can_104.h"
#include "isc_104.h"
#include "spi_104.h"

int main() {
  // I2C command
  send_i2c_message();
  // SPI command
  send_spi_message();

  return 0;
}
