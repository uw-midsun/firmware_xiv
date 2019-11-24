#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "spi.h"
#include "stdbool.h"

#define WAKEUP 0x2
#define SLEEP 0x4
#define RESET 0x6
#define START 0x8
#define STOP 0xA
#define RDATAC 0x10
#define SDATAC 0x11
#define RDATA 0x12
#define RREG_BASE 0x20
#define WREG_BASE 0x40
#define OFSCAL 0x18
#define GANCAL 0x19

void read_reg(uint8_t offset, uint8_t num_regs, uint8_t *data) {
  uint8_t tx_len = 2;
  uint8_t read_cmd = RREG_BASE | offset;
  uint8_t op_code[2] = { read_cmd, num_regs - 1 };
  spi_exchange(SPI_PORT_2, op_code, tx_len, data, num_regs);
}

int main() {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  GpioAddress cs = { .port = GPIO_PORT_B, .pin = 12 };
  SpiSettings spi_settings = { .baudrate = 1000000,
                               .mode = SPI_MODE_0,
                               .mosi = { .port = GPIO_PORT_B, .pin = 15 },
                               .miso = { .port = GPIO_PORT_B, .pin = 14 },
                               .sclk = { .port = GPIO_PORT_B, .pin = 13 },
                               .cs = cs };
  spi_init(SPI_PORT_2, &spi_settings);

  while (true) {
    uint8_t reg_offset = 0;
    uint8_t data[1] = { 0xa };
    read_reg(0, 1, data);
    LOG_DEBUG("REG %d: %x\n", reg_offset, data[0]);
    delay_ms(200);
  }
  return 0;
}
