// Simple smoketest project for the solar boards
// specifically for the spv1020s

#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "mppt.h"
#include "soft_timer.h"
#include "spi.h"

#define SPI_PORT SPI_PORT_2
#define SPV1020 0

#define SMOKETEST_WAIT_TIME_MS 1000

#define BAUDRATE 60000
#define MOSI_PIN \
  { .port = GPIO_PORT_B, 15 }
#define MISO_PIN \
  { .port = GPIO_PORT_B, 14 }
#define SCLK_PIN \
  { .port = GPIO_PORT_B, 13 }
#define CS_PIN \
  { .port = GPIO_PORT_B, 12 }

uint8_t status = 0xFF;      // 8th bit is set
uint16_t pwm = 0xFFFF;      // over 900
uint16_t vin = 0xFFFF;      // over 10 bits
uint16_t current = 0xFFFF;  // over 10 bits

static void prv_spv1020_check(SoftTimerId timer_id, void *context) {
  status = 0xFF;
  pwm = 0xFFFF;
  vin = 0xFFFF;
  current = 0xFFFF;
  mppt_turn_on(SPI_PORT, SPV1020);

  mppt_read_status(SPI_PORT, &status, SPV1020);
  LOG_DEBUG("SPV1020 # %d status is: %x\r\n", SPV1020, status);
  mppt_read_pwm(SPI_PORT, &pwm, SPV1020);
  LOG_DEBUG("SPV1020 # %d pwm is: %x\r\n", SPV1020, pwm);
  mppt_read_voltage_in(SPI_PORT, &vin, SPV1020);
  LOG_DEBUG("SPV1020 # %d status is: %x\r\n", SPV1020, vin);
  mppt_read_current(SPI_PORT, &current, SPV1020);
  LOG_DEBUG("SPV1020 # %d status is: %x\r\n", SPV1020, current);

  mppt_shut(SPI_PORT, SPV1020);
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
    .mosi = MOSI_PIN,
    .miso = MISO_PIN,
    .sclk = SCLK_PIN,
    .cs = CS_PIN,
  };
  spi_init(SPI_PORT, &spi_settings);

  mppt_init();

  LOG_DEBUG("Initializing spv1020 smoke test\n");

  Event e = { 0 };
  soft_timer_start_millis(SMOKETEST_WAIT_TIME_MS, prv_spv1020_check, NULL, NULL);
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
  }
  return 0;
}
