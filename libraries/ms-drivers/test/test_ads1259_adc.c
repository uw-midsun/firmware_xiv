#include "ads1259_adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "math.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_AD1259_SPI_PORT SPI_PORT_2
#define NUM_CONFIG_REGISTERS 3
#define TEST_CHK_SUM_FLAG_BIT 0x80
#define TEST_DATA_SETTLING_TIME_MS 17

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

typedef enum Ads1259TestMode {
  ADS1259_MODE_MAX_DATA = 0,
  ADS1259_MODE_MIXED_DATA,
  ADS1259_MODE_CHECKSUM_FAULT,
  ADS1259_MODE_OUT_RANGE_FLAG_TRIGGERED,
  NUM_TEST_MODES,
} Ads1259TestMode;

static Ads1259Storage storage;
static Ads1259MockRegisters registers;
static Ads1259TestMode test_mode;
static bool checksum;
static bool out_of_range;

static void prv_spi_return_conv_data(uint8_t *rx_data, Ads1259TestMode mode) {
  switch (mode) {
    case ADS1259_MODE_MAX_DATA:
      rx_data[0] = 0xFF;
      rx_data[1] = 0xFF;
      rx_data[2] = 0xFF;
      rx_data[3] = (uint8_t)((0x2FD + ADS1259_CHECKSUM_OFFSET) & 0x7F);
      break;
    case ADS1259_MODE_MIXED_DATA:
      rx_data[0] = 0x10;
      rx_data[1] = 0x20;
      rx_data[2] = 0x30;
      rx_data[3] = (uint8_t)((0x60 + ADS1259_CHECKSUM_OFFSET) & 0x7F);
      break;
    case ADS1259_MODE_CHECKSUM_FAULT:
      rx_data[0] = 0x10;
      rx_data[1] = 0x20;
      rx_data[2] = 0x30;
      rx_data[3] = (uint8_t)((0x59 + ADS1259_CHECKSUM_OFFSET) & 0x7F);
      break;
    case ADS1259_MODE_OUT_RANGE_FLAG_TRIGGERED:
      rx_data[0] = 0x10;
      rx_data[1] = 0x20;
      rx_data[2] = 0x30;
      rx_data[3] = 0x80;
      break;
    default:
      break;
  }
}

static void prv_error_handler(Ads1259StatusCode code, void *context) {
  if (code == ADS1259_STATUS_CODE_OUT_OF_RANGE) out_of_range = true;
  if (code == ADS1259_STATUS_CODE_CHECKSUM_FAULT) checksum = true;
}

StatusCode TEST_MOCK(spi_exchange)(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                                   size_t rx_len) {
  TEST_ASSERT_EQUAL(spi, TEST_AD1259_SPI_PORT);
  uint8_t cmd = tx_data[0];
  if (cmd & ADS1259_READ_REGISTER) {
    if (cmd == 0x20) rx_data[0] = registers.CONFIG_0;
    if (cmd == 0x21) rx_data[0] = registers.CONFIG_1;
    if (cmd == 0x22) rx_data[0] = registers.CONFIG_2;
    return STATUS_CODE_OK;
  }
  switch (cmd) {
    // Commands used in ads1259_init()
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
    // Commands used in Ads1259_get_conversion_data()
    case ADS1259_START_CONV:
      LOG_DEBUG("CONVERSIONS STARTED \n");
      break;
    case ADS1259_READ_DATA_BY_OPCODE:
      LOG_DEBUG("SETTING CONVERSION DATA\n");
      prv_spi_return_conv_data(rx_data, test_mode);
      break;
  }
  return STATUS_CODE_OK;
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
  // Test that all registers have been configured correctly and setup has worked properly
  TEST_ASSERT_OK(ads1259_init(&settings, &storage));
  TEST_ASSERT_EQUAL((registers.FSC_0 | registers.FSC_1 | registers.FSC_2), 0);
  TEST_ASSERT_EQUAL((registers.OFC_0 | registers.OFC_1 | registers.OFC_2), 0);
  TEST_ASSERT_EQUAL(registers.CONFIG_0, register_lookup[0]);
  TEST_ASSERT_EQUAL(registers.CONFIG_1, register_lookup[1]);
  TEST_ASSERT_EQUAL(registers.CONFIG_2, register_lookup[2]);
}

void teardown_test(void) {}

void test_ads1259_get_conversion_data() {
  // test with max data
  test_mode = ADS1259_MODE_MAX_DATA;
  uint32_t test_raw = 0xFFFFFF;
  ads1259_get_conversion_data(&storage);
  delay_ms(TEST_DATA_SETTLING_TIME_MS);
  TEST_ASSERT_EQUAL(storage.conv_data.MSB | storage.conv_data.MID | storage.conv_data.LSB, 0xFF);
  TEST_ASSERT_EQUAL(storage.conv_data.raw, test_raw);
  TEST_ASSERT_EQUAL(storage.reading, (test_raw >> 4) * EXTERNAL_VREF / pow(2, 20));
  // TEST_ASSERT_EQUAL(storage.reading, 50.0);

  // test with a random data set
  test_mode = ADS1259_MODE_MIXED_DATA;
  test_raw = 0x302010;
  ads1259_get_conversion_data(&storage);
  delay_ms(TEST_DATA_SETTLING_TIME_MS);
  TEST_ASSERT_EQUAL(storage.conv_data.MSB | storage.conv_data.MID | storage.conv_data.LSB, 0x30);
  TEST_ASSERT_EQUAL(storage.conv_data.raw, test_raw);
  TEST_ASSERT_EQUAL(storage.reading, (test_raw >> 4) * EXTERNAL_VREF / pow(2, 20));
  TEST_ASSERT_EQUAL(storage.reading, 9.399462);

  // test checksum fault triggered
  test_mode = ADS1259_MODE_CHECKSUM_FAULT;
  checksum = false;
  ads1259_get_conversion_data(&storage);
  delay_ms(TEST_DATA_SETTLING_TIME_MS);
  TEST_ASSERT_TRUE(checksum);

  // test out of range flag triggered
  test_mode = ADS1259_MODE_OUT_RANGE_FLAG_TRIGGERED;
  out_of_range = false;
  ads1259_get_conversion_data(&storage);
  delay_ms(TEST_DATA_SETTLING_TIME_MS);
  TEST_ASSERT_TRUE(out_of_range);
}
