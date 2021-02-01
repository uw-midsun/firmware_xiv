#include "fw_104_can.h"
#include "fw_104_i2c.h"
#include "fw_104_spi.h"

int main() {
  prv_write_A_message();
  prv_write_B_message();
  return 0;
}
