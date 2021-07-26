

#include "gpio.h"
#include "soft_timer.h"
#include "status.h"
#include "spi.h"
#include "adc.h"
#include "log.h"
#include "status.h"
#include "interrupt.h"
#include "delay.h"

#include "mcp2515.h"

static Mcp2515Storage s_mcp2515;

// static void prv_read(Mcp2515Storage *storage, uint8_t addr, uint8_t *read_data, size_t read_len) {
//   uint8_t payload[] = { MCP2515_CMD_READ, addr };
//   spi_exchange(storage->spi_port, payload, sizeof(payload), read_data, read_len);
// }

// static Mcp2515Settings mcp2515_settings = {
//  .spi_port = SPI_PORT_2,                        //
//  .spi_baudrate = 6000000,                       //
//  .mosi = { .port = GPIO_PORT_B, .pin = 15 },    //
//  .miso = { .port = GPIO_PORT_B, .pin = 14 },    //
//  .sclk = { .port = GPIO_PORT_B, .pin = 13 },    //
//  .cs = { .port = GPIO_PORT_B, .pin = 12 },      //
//  //.int_pin = { .port = GPIO_PORT_A, .pin = 8 },  //
//  .can_bitrate = MCP2515_BITRATE_250KBPS,        //
//  .loopback = false,                             //
// };

int main(void) {
  gpio_init();
  interrupt_init();
  adc_init(ADC_MODE_SINGLE);
  soft_timer_init();
  
const SpiSettings spi_settings = {
  .baudrate = 6000000,                       //
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },    //
  .miso = { .port = GPIO_PORT_B, .pin = 14 },    //
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },    //
  .cs = { .port = GPIO_PORT_B, .pin = 12 },  
  };
  LOG_DEBUG("MCP INITING\n");
  //mcp2515_init(&s_mcp2515, &mcp2515_settings);
  status_ok_or_return(spi_init(SPI_PORT_2, &spi_settings));

  uint8_t payload[] = { MCP2515_CMD_RESET };
  spi_exchange(SPI_PORT_2, payload, sizeof(payload), NULL, 0);
  delay_us(100);

  LOG_DEBUG("MCP DONE INITING\n");
  uint8_t read_data = 0;
  //prv_read(&s_mcp2515, MCP2515_CTRL_REG_CNF3, &read_data, 1);
  uint8_t payload1[] = { MCP2515_CMD_READ, MCP2515_CTRL_REG_CNF3 };
  spi_exchange(s_mcp2515.spi_port, payload1, sizeof(payload), &read_data, 1);
  LOG_DEBUG("REG VALUE: %d\n", read_data);


  return 0;
}
