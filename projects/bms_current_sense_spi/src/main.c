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

static void send_only_cmd(uint8_t cmd) {
  spi_exchange(SPI_PORT_2, &cmd, 1, NULL, 0);
}

typedef enum {
  REG_CONFIG_0 = 0,
  REG_CONFIG_1,
  REG_CONFIG_2,
  NUM_REG_CONFIGS
} ConfigRegister;

static char* s_reg_name_lookup[NUM_REG_CONFIGS] = {
  [REG_CONFIG_0] = "Config 0",
  [REG_CONFIG_1] = "Config 1",
  [REG_CONFIG_2] = "Config 2"
};

void read_reg(uint8_t offset, uint8_t num_regs, uint8_t *data) {
  uint8_t tx_len = 2;
  uint8_t read_cmd = RREG_BASE | offset;
  uint8_t op_code[2] = { read_cmd, num_regs - 1 };
  spi_exchange(SPI_PORT_2, op_code, tx_len, data, num_regs);
}

void read_one_reg(uint8_t reg_offset) {
  uint8_t data = 0;
  read_reg(reg_offset, 1, &data);
  //LOG_DEBUG("%s: %x\n", s_reg_name_lookup[reg_offset], data);
}

void read_all_regs() {
  uint8_t i;
  for (i = 0; i < NUM_REG_CONFIGS; i++) {
    ConfigRegister reg = (ConfigRegister) i;
    read_one_reg(reg);
  }
}

void start_conversion() {
  send_only_cmd(START);
}

void set_read_data_continuous() {
  LOG_DEBUG("[CONF] read continuous\n");
  send_only_cmd(RDATAC);
}

void reset() {
  send_only_cmd(RESET);
}

void sleep() {
  send_only_cmd(SLEEP);
}

void wake_up() {
  //LOG_DEBUG("[CMD]: Wake Up\n");
  send_only_cmd(WAKEUP);
}

void sleep_then_wakeup() {
  sleep();
  wake_up();
  delay_us(80);
}

void setup() {
  LOG_DEBUG("Wait for power up\n");
  delay_ms(10);

  LOG_DEBUG("Setting UP\n");
  sleep_then_wakeup();
  reset();
  //set_read_data_continuous();
}

void read_current() {
    uint8_t read_cmd = RDATA;
    uint8_t current[3] = { 0 };
    spi_exchange(SPI_PORT_2, &read_cmd, 1, current, 3);
    uint32_t c = (uint32_t)((current[0] << 16) | (current[1] << 8) | current[2]);
    //LOG_DEBUG("current: %lu\n", c);
}


int main() {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  GpioAddress cs = { .port = GPIO_PORT_B, .pin = 12 };
  SpiSettings spi_settings = { .baudrate = 750000,
                               .mode = SPI_MODE_1,
                               .mosi = { .port = GPIO_PORT_B, .pin = 15 },
                               .miso = { .port = GPIO_PORT_B, .pin = 14 },
                               .sclk = { .port = GPIO_PORT_B, .pin = 13 },
                               .cs = cs };
  spi_init(SPI_PORT_2, &spi_settings);

  LOG_DEBUG("Hello\n");

  setup();
  //read_all_regs();

  while (true) {
    //wake_up();
    //read_current();
    read_one_reg(REG_CONFIG_0);
  }
  return 0;
}
