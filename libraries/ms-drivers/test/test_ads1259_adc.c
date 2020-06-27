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
  interrupt_init();
  gpio_init();
  soft_timer_init();
  TEST_ASSERT_OK(ads1259_init(&settings, &storage));
}

void teardown_test() {}

void test_setup() {
  TEST_ASSERT_TRUE(storage.spi_port == TEST_SPI_PORT);
  TEST

}

void test_conversions() {
  uint32_t data_storage[10];
  LOG_DEBUG("TESTING COVERSIONS...\n");
  TEST_ASSERT_OK(ads1259_get_conversion_data(&storage));
  // Makes sure that out of range flagged data is not making it through
  TEST_ASSERT_TRUE((storage.data.ADS_RX_CHK_SUM & CHK_SUM_FLAG_BIT) == 0);
  // Checks that reading values output are all 24 bits
  TEST_ASSERT_TRUE(storage.reading >> 24 == 0);
  // test rapid fire sequential readings
  for (int i = 0; i < 10; i++) {
    TEST_ASSERT_OK(ads1259_get_conversion_data(&storage));
    data_storage[i] = storage.reading;
  }
  // Check logs to make sure data is not garbled
  for (int i = 0; i < 10; i++) {
    LOG_DEBUG("%lu\n", data_storage[i]);
  }
}
