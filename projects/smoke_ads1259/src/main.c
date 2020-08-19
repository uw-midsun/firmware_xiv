#include <stdlib.h>

#include "ads1259_adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// change this number to read for x cycles, 0 for infinite
#define READ_CYCLE_NUM 2
// change this number to read x times in a cycle
#define READ_CYCLE_SIZE 3

// slightly larger than conversion time of adc
#define CONVERSION_TIME_MS 18
#define READING_QUEUE_LENGTH READ_CYCLE_SIZE
static Ads1259Storage s_storage;
static uint8_t s_index;
static uint8_t s_count = 1;

static void prv_rx_error_handler_cb(Ads1259StatusCode code, void *context) {
  if (code == ADS1259_STATUS_CODE_OUT_OF_RANGE) {
    LOG_DEBUG("ERROR: OUT OF RANGE\n");
  } else if (code == ADS1259_STATUS_CODE_CHECKSUM_FAULT) {
    LOG_DEBUG("ERROR: CHECKSUM FAULT\n");
  }
}

static void prv_periodic_read(SoftTimerId id, void *context) {
  double *queue = context;
  if (s_index < READING_QUEUE_LENGTH) {
    LOG_DEBUG("=========READING # %i========= vref mv: %d\n", s_count++,
              (int)(EXTERNAL_VREF_V * 1000));
    ads1259_get_conversion_data(&s_storage);
    queue[s_index] = s_storage.reading;
    LOG_DEBUG("%d mV", (uint16_t)(s_storage.reading * 1000));
    s_index++;
    soft_timer_start_millis(CONVERSION_TIME_MS, prv_periodic_read, queue, NULL);
  } else {
    if (s_count <= READ_CYCLE_NUM * READ_CYCLE_SIZE || READ_CYCLE_NUM == 0) {
      s_index = 0;
      soft_timer_start_millis(1000, prv_periodic_read, queue, NULL);
    } else {
      exit(0);
    }
  }
}

int main() {
  const Ads1259Settings settings = {
    .spi_port = SPI_PORT_2,
    .spi_baudrate = 600000,
    .mosi = { .port = GPIO_PORT_B, 15 },
    .miso = { .port = GPIO_PORT_B, 14 },
    .sclk = { .port = GPIO_PORT_B, 13 },
    .cs = { .port = GPIO_PORT_B, 12 },
    .handler = prv_rx_error_handler_cb,
  };
  interrupt_init();
  gpio_init();
  soft_timer_init();
  LOG_DEBUG("Smoke test initializing ads1259\n");
  delay_ms(100);
  ads1259_init(&s_storage, &settings);

  s_index = 0;
  double reading_queue[READING_QUEUE_LENGTH];

  soft_timer_start_millis(CONVERSION_TIME_MS, prv_periodic_read, reading_queue, NULL);

  while (1) {
  }
}
