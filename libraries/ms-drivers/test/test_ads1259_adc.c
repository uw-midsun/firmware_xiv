#include "ads1259_adc.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"


#define TEST_AD1259_SPI_PORT SPI_PORT_2
#define NUM_CONFIG_REGISTERS 3
#define TEST_CHK_SUM_FLAG_BIT 0x80
#define TEST_DRDY_BIT 0x80



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
    (ADS1259_SPI_TIMEOUT_ENABLE),
    (ADS1259_OUT_OF_RANGE_FLAG_ENABLE | ADS1259_CHECK_SUM_ENABLE),
    (ADS1259_CONVERSION_CONTROL_MODE_PULSE | ADS1259_DATA_RATE_SPS),
  };

static Ads1259Storage storage;
static Ads1259MockRegisters registers;

StatusCode TEST_MOCK(spi_exchange)(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                                   size_t rx_len) {
  TEST_ASSERT_EQUAL(spi, TEST_AD1259_SPI_PORT);
  uint8_t cmd = tx_data[0];
  LOG_DEBUG("%x\n",cmd);
  if(cmd & ADS1259_READ_REGISTER) {
    if(cmd == 0x20)
      rx_data[0] = registers.CONFIG_0;
    if(cmd == 0x21)
      rx_data[0] = registers.CONFIG_1;
    if(cmd == 0x22)
      rx_data[0] = registers.CONFIG_2;
    return STATUS_CODE_OK;
  }
  switch (cmd) {
    //Commands used in ads1259_init()
    case ADS1259_STOP_READ_DATA_CONTINUOUS:
      LOG_DEBUG("STOP DATA CMD SENT\n");
        return STATUS_CODE_OK;
    case ADS1259_RESET:
      LOG_DEBUG("RESET CMD SENT\n");
        return STATUS_CODE_OK;
    case (ADS1259_WRITE_REGISTER | ADS1259_ADDRESS_CONFIG0):
      LOG_DEBUG("WRITE_REG CMD SENT\n");
      TEST_ASSERT_TRUE(tx_len == 5 && rx_len == 0);
      TEST_ASSERT_TRUE(tx_data[1] == 3);
      registers.CONFIG_0 = tx_data[2];
      registers.CONFIG_1 = tx_data[3];
      registers.CONFIG_2 = tx_data[4];
      break;
    case ADS1259_OFFSET_CALIBRATION:
      LOG_DEBUG("OFFSET CALIBRATION SENT\n");
      registers.OFC_0 = 0;
      registers.OFC_1 = 0;
      registers.OFC_2 = 0;
      break;
    case ADS1259_GAIN_CALIBRATION:
      LOG_DEBUG("GAIN CALIBRATION SENT\n");
      registers.FSC_0 = 0;
      registers.FSC_1 = 0;
      registers.FSC_2 = 0;
      break;
    case ADS1259_START_CONV:
      LOG_DEBUG("CONVERSIONS STARTED \n");
      break;
    case ADS1259_READ_DATA_BY_OPCODE:
      LOG_DEBUG("SETTING CONVERSION DATA\n");
      rx_data[0] = 0x10;
      rx_data[1] = 0x10;
      rx_data[2] = 0x10;
      rx_data[3] = 0x30;
      break;
  } 
  return STATUS_CODE_OK;
}

static void prv_error_handler(Ads1259StatusCode code, void* context) {
  if(code == ADS1259_STATUS_CODE_OUT_OF_RANGE)
    LOG_DEBUG("ADC READING IS OUT OF RANGE\n");
  if(code == ADS1259_STATUS_CODE_CHECKSUM_FAULT)
    LOG_DEBUG("CHECKSUM INVALID\n");
}

void setup_test() {
  Ads1259Settings settings = {
    .spi_port = TEST_AD1259_SPI_PORT,
    .spi_baudrate = TEST_BAUDRATE,
    .mosi = TEST_MOSI_PIN,
    .miso = TEST_MISO_PIN,
    .sclk = TEST_SCLK_PIN,
    .cs = TEST_CS_PIN,
    .handler = prv_error_handler,
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
  TEST_ASSERT_OK(ads1259_init(&settings, &storage));
  TEST_ASSERT_TRUE((registers.FSC_0 | registers.FSC_1 | registers.FSC_2) == 0);
  TEST_ASSERT_TRUE((registers.OFC_0 | registers.OFC_1 | registers.OFC_2) == 0);
  TEST_ASSERT_TRUE(registers.CONFIG_0 == register_lookup[0]);
  TEST_ASSERT_TRUE(registers.CONFIG_1 == register_lookup[1]);
  TEST_ASSERT_TRUE(registers.CONFIG_2 == register_lookup[2]);
}

void teardown_test(void) {}

static void super_cb(SoftTimerId timer_id, void* context) {
  LOG_DEBUG("CALLBACK SUPER CALLED\n");
}

void test_ads1259_init() {
  soft_timer_start_millis(17, super_cb, NULL, NULL);
  ads1259_get_conversion_data(&storage);
}


