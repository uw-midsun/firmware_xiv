#include "ads1259_adc.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define CHK_SUM_FLAG_BIT 0x80
#define TEST_AD1259_SPI_PORT SPI_PORT_2
#define NUM_CONFIG_REGISTERS 3


#define TEST_BAUDRATE 60000
#define TEST_MOSI_PIN \
  { .port = GPIO_PORT_B, 15 }
#define TEST_MISO_PIN \
  { .port = GPIO_PORT_B, 14 }
#define TEST_SCLK_PIN \
  { .port = GPIO_PORT_B, 13 }
#define TEST_CS_PIN \
  { .port = GPIO_PORT_B, 12 }

typedef struct Ads1259MockRegisters {
  uint8_t CONFIG_0;
  uint8_t CONFIG_1;
  uint8_t CONFIG_2;
  uint8_t OFC_0;
  uint8_t OFC_1;
  uint8_t OFC_2;
  uint8_t FSC_0;
  uint8_t FSC_1;
  uint8_t FSC_2; 
} Ads1259MockRegisters;

static uint8_t register_lookup[NUM_CONFIG_REGISTERS] = {
    (ADS1259_INTERNAL_REF_BIAS_ENABLE | ADS1259_SPI_TIMEOUT_ENABLE),
    (ADS1259_OUT_OF_RANGE_FLAG_ENABLE | ADS1259_CHECK_SUM_ENABLE),
    (ADS1259_CONVERSION_CONTROL_MODE_PULSE | ADS1259_DATA_RATE_SPS),
  };

static Ads1259Storage storage;
static Ads1259MockRegisters registers;

void setup_test() {
  Ads1259Settings settings = {
    .spi_port = TEST_AD1259_SPI_PORT,
    .spi_baudrate = TEST_BAUDRATE,
    .mosi = TEST_MOSI_PIN,
    .miso = TEST_MISO_PIN,
    .sclk = TEST_SCLK_PIN,
    .cs = TEST_CS_PIN,
  };
  registers.CONFIG_0 = 0;
  registers.CONFIG_1 = 0;
  registers.CONFIG_2 = 0;
  registers.OFC_0 = 1;
  registers.OFC_1 = 1;
  registers.OFC_2 = 1;
  registers.FSC_0 = 1;
  registers.FSC_1 = 1;
  registers.FSC_2 = 1;
  interrupt_init();
  gpio_init();
  soft_timer_init();
  ads1259_init(&settings, &storage);
}

void teardown_test() {}

StatusCode TEST_MOCK(spi_exchange)(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                                   size_t rx_len) {
  TEST_ASSERT_TRUE(rx_len == SIZEOF_ARRAY(rx_data) || tx_len == SIZEOF_ARRAY(tx_data));
  TEST_ASSERT_EQUAL(spi, TEST_AD1259_SPI_PORT);
  uint16_t cmd = tx_data[0];
  if(cmd & ADS1259_READ_REGISTER) {
    if(cmd & 0x0)
      storage.rx_data.MSB = registers.CONFIG_0;
    if(cmd & 0x1)
      storage.rx_data.MSB = registers.CONFIG_1;
    if(cmd & 0x2)
      storage.rx_data.MSB = registers.CONFIG_0;
    return STATUS_CODE_OK;
  }
  switch (cmd) {
    case ADS1259_STOP_READ_DATA_CONTINUOUS:
        return STATUS_CODE_OK;
    case ADS1259_RESET:
        return STATUS_CODE_OK;
    case (ADS1259_WRITE_REGISTER | ADS1259_ADDRESS_CONFIG0):
      TEST_ASSERT_TRUE(tx_len == 5 && rx_len == 0);
      TEST_ASSERT_TRUE(tx_data[1] == 3);
      registers.CONFIG_0 = tx_data[2];
      registers.CONFIG_1 = tx_data[3];
      registers.CONFIG_2 = tx_data[4];
        break;
    case ADS1259_OFFSET_CALIBRATION:
      registers.OFC_0 = 0;
      registers.OFC_1 = 0;
      registers.OFC_2 = 0;
      break;
    case ADS1259_GAIN_CALIBRATION:
      registers.FSC_0 = 0;
      registers.FSC_1 = 0;
      registers.FSC_2 = 0;
      break;
  }
  return STATUS_CODE_OK;
}

void test_ads1259_init() {
  LOG_DEBUG("%d", registers.CONFIG_0);
  LOG_DEBUG("%x", registers.CONFIG_0);
}


