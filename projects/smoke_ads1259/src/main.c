#include "ads1259_adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// slightly larger than conversion time of adc
#define CONVERSION_TIME_MS 18
#define READING_QUEUE_LENGTH 3
static Ads1259Storage s_storage;
static int s_index;
static int s_count = 1;

static void prv_rx_error_handler_cb(Ads1259StatusCode code, void *context) {
  if (code == ADS1259_STATUS_CODE_OUT_OF_RANGE) {
    LOG_DEBUG("ERROR: OUT OF RANGE\n");
  } else if (code == ADS1259_STATUS_CODE_CHECKSUM_FAULT) {
    LOG_DEBUG("ERROR: CHECKSUM FAULT\n");
  }
}

// static void prv_dump_queue(double *queue) {
//   LOG_DEBUG("DUMPING QUEUE....\n");
//   for (int reading = 0; reading < READING_QUEUE_LENGTH; reading++) {
//     LOG_DEBUG("VOLTAGE READING: %lf\n", queue[reading]);
//     queue[reading] = 0;
//   }
//   LOG_DEBUG("----------DONE DUMPING---------\n");
// }

static void prv_periodic_read(SoftTimerId id, void *context) {
  double *queue = context;
  if (s_index < READING_QUEUE_LENGTH) {
    printf("=========READING # %i========= vref mv: %d\n", s_count++, (int)(EXTERNAL_VREF_V * 1000));
    ads1259_get_conversion_data(&s_storage);
    queue[s_index] = s_storage.reading;
    s_index++;
    soft_timer_start_millis(CONVERSION_TIME_MS, prv_periodic_read, queue, NULL);
  } else {
    // prv_dump_queue(queue);
    if (s_count > 6) {
      return;
    }
    s_index = 0;
    soft_timer_start_millis(1000, prv_periodic_read, queue, NULL);
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
  printf("smoke test initializing ads1259\n");
  delay_ms(100);
  ads1259_init(&settings, &s_storage);

  s_index = 0;
  double reading_queue[READING_QUEUE_LENGTH];

  // [jess] just do it twice
  // prv_periodic_read(SOFT_TIMER_INVALID_TIMER, reading_queue);
  // printf("setting timer for another read...\n");
  soft_timer_start_millis(CONVERSION_TIME_MS, prv_periodic_read, reading_queue, NULL);

  while (1) {
    // soft_timer_start_millis(CONVERSION_TIME_MS, prv_periodic_read, reading_queue, NULL);
    // delay_ms(1000);
  }
}
