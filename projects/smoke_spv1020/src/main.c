// Simple smoketest project for the solar boards
// specifically for the spv1020s

#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "spi.h"
#include "mppt.h"

#define TEST_SPI_PORT SPI_PORT_2

#define TEST_BAUDRATE 60000
#define TEST_MOSI_PIN \
  { .port = GPIO_PORT_B, 15 }
#define TEST_MISO_PIN \
  { .port = GPIO_PORT_B, 14 }
#define TEST_SCLK_PIN \
  { .port = GPIO_PORT_B, 13 }
#define TEST_CS_PIN \
  { .port = GPIO_PORT_B, 12 }

static void prv_spv1020_check(SoftTimerId timer_id, void *context) {
  
}

int main(void) {
  event_queue_init();
  gpio_init();
  gpio_it_init();
  interrupt_init();
  soft_timer_init();

  SpiSettings spi_settings = {
    .baudrate = 60000,
    .mode = SPI_MODE_3,
    .mosi = TEST_MOSI_PIN,
    .miso = TEST_MISO_PIN,
    .sclk = TEST_SCLK_PIN,
    .cs = TEST_CS_PIN,
  };
  spi_init(TEST_SPI_PORT, &spi_settings);

  mppt_init();

  LOG_DEBUG("Initializing spv1020 smoke test\n");

  Event e = { 0 };
  soft_timer_start_millis(500, prv_spv1020_check, NULL, NULL);
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
  }
  return 0;
}
