#include "ads1259_adc.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define CHK_SUM_FLAG_BIT 0x80
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

static Ads1259Storage storage;

void setup_test() {
  Ads1259Settings settings = {
    .spi_port = TEST_SPI_PORT,
    .spi_baudrate = TEST_BAUDRATE,
    .mosi = TEST_MOSI_PIN,
    .miso = TEST_MISO_PIN,
    .sclk = TEST_SCLK_PIN,
    .cs = TEST_CS_PIN,
  };
  gpio_init();
  soft_timer_init();
  ads1259_init(&settings, &storage);
}

void teardown_test() {}

void test_conversions() {

}
