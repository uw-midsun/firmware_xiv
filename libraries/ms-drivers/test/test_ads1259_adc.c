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
  ADS1259_MODE_MAX_POS_DATA = 0,
  ADS1259_MODE_MIN_POS_DATA,
  ADS1259_MODE_MAX_NEG_DATA,
  ADS1259_MODE_MIN_NEG_DATA,
  ADS1259_MODE_ZERO_DATA,
  ADS1259_MODE_MIXED_DATA,
  ADS1259_MODE_CHECKSUM_FAULT,
  ADS1259_MODE_OUT_RANGE_FLAG_TRIGGERED,
  NUM_TEST_MODES,
} Ads1259TestMode;

static Ads1259Storage s_storage;
static Ads1259MockRegisters s_registers;
static Ads1259TestMode s_test_mode;
static bool s_checksum;
static bool s_out_of_range;

static void prv_spi_return_conv_data(uint8_t *rx_data, Ads1259TestMode mode) {
  switch (mode) {
    case ADS1259_MODE_MIN_POS_DATA:
      rx_data[0] = 0x00;
      rx_data[1] = 0x00;
      rx_data[2] = 0x10;
      rx_data[3] = (uint8_t)((0x27D + ADS1259_CHECKSUM_OFFSET) & 0x7F);
      break;
    case ADS1259_MODE_MIN_NEG_DATA:
      rx_data[0] = 0xFF;
      rx_data[1] = 0xFF;
      rx_data[2] = 0xEF;
      rx_data[3] = (uint8_t)((0x80 + ADS1259_CHECKSUM_OFFSET) & 0x7F);
      break;
    case ADS1259_MODE_MAX_POS_DATA:
      rx_data[0] = 0x7F;
      rx_data[1] = 0xFF;
      rx_data[2] = 0xFF;
      rx_data[3] = (uint8_t)((0x27D + ADS1259_CHECKSUM_OFFSET) & 0x7F);
      break;
    case ADS1259_MODE_MAX_NEG_DATA:
      rx_data[0] = 0x80;
      rx_data[1] = 0x00;
      rx_data[2] = 0x00;
      rx_data[3] = (uint8_t)((0x80 + ADS1259_CHECKSUM_OFFSET) & 0x7F);
      break;
    case ADS1259_MODE_ZERO_DATA:
      rx_data[0] = 0x00;
      rx_data[1] = 0x00;
      rx_data[2] = 0x00;
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
  if (code == ADS1259_STATUS_CODE_OUT_OF_RANGE) {
    s_out_of_range = true;
  }
  if (code == ADS1259_STATUS_CODE_CHECKSUM_FAULT) {
    s_checksum = true;
  }
}

StatusCode TEST_MOCK(spi_exchange)(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                                   size_t rx_len) {
  TEST_ASSERT_EQUAL(spi, TEST_AD1259_SPI_PORT);
  uint8_t cmd = tx_data[0];
  if (cmd & ADS1259_READ_REGISTER) {
    if (cmd == 0x20) rx_data[0] = s_registers.CONFIG_0;
    if (cmd == 0x21) rx_data[0] = s_registers.CONFIG_1;
    if (cmd == 0x22) rx_data[0] = s_registers.CONFIG_2;
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
      TEST_ASSERT_TRUE(tx_data[1] == 2);
      s_registers.CONFIG_0 = tx_data[2];
      s_registers.CONFIG_1 = tx_data[3];
      s_registers.CONFIG_2 = tx_data[4];
      break;
    case ADS1259_OFFSET_CALIBRATION:
      LOG_DEBUG("OFFSET CALIBRATION SENT\n");
      s_registers.OFC_0 = 0;
      s_registers.OFC_1 = 0;
      s_registers.OFC_2 = 0;
      break;
    case ADS1259_GAIN_CALIBRATION:
      LOG_DEBUG("GAIN CALIBRATION SENT\n");
      s_registers.FSC_0 = 0;
      s_registers.FSC_1 = 0;
      s_registers.FSC_2 = 0;
      break;
    // Commands used in Ads1259_get_conversion_data()
    case ADS1259_START_CONV:
      break;
    case ADS1259_READ_DATA_BY_OPCODE:
      prv_spi_return_conv_data(rx_data, s_test_mode);
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
  s_registers.CONFIG_0 = 0;
  s_registers.CONFIG_1 = 0;
  s_registers.CONFIG_2 = 0;
  s_registers.OFC_0 = 1;
  s_registers.OFC_1 = 1;
  s_registers.OFC_2 = 1;
  s_registers.FSC_0 = 1;
  s_registers.FSC_1 = 1;
  s_registers.FSC_2 = 1;
  interrupt_init();
  gpio_init();
  soft_timer_init();
  // Test that all registers have been configured correctly and setup has worked properly
  TEST_ASSERT_OK(ads1259_init(&settings, &s_storage));
  TEST_ASSERT_EQUAL(0, (s_registers.FSC_0 | s_registers.FSC_1 | s_registers.FSC_2));
  TEST_ASSERT_EQUAL(0, (s_registers.OFC_0 | s_registers.OFC_1 | s_registers.OFC_2));
  TEST_ASSERT_EQUAL(register_lookup[0], s_registers.CONFIG_0);
  TEST_ASSERT_EQUAL(register_lookup[1], s_registers.CONFIG_1);
  TEST_ASSERT_EQUAL(register_lookup[2], s_registers.CONFIG_2);
}

void teardown_test(void) {}

void test_ads1259_get_conversion_data() {
  // test with max pos data
  s_test_mode = ADS1259_MODE_MAX_POS_DATA;
  uint32_t test_raw = 0x7FFFFF;
  ads1259_get_conversion_data(&s_storage);
  delay_ms(TEST_DATA_SETTLING_TIME_MS);
  TEST_ASSERT_EQUAL(0xFF, s_storage.conv_data.LSB | s_storage.conv_data.MID);
  TEST_ASSERT_EQUAL(0x7F, s_storage.conv_data.MSB);
  TEST_ASSERT_EQUAL(test_raw, s_storage.conv_data.raw);
  TEST_ASSERT_EQUAL((test_raw >> 4) * EXTERNAL_VREF_V / (pow(2, 19) - 1), s_storage.reading);
  TEST_ASSERT_EQUAL(50, s_storage.reading);

  // test with max neg data
  s_test_mode = ADS1259_MODE_MAX_NEG_DATA;
  test_raw = 0x800000;
  ads1259_get_conversion_data(&s_storage);
  delay_ms(TEST_DATA_SETTLING_TIME_MS);
  TEST_ASSERT_EQUAL(-50, s_storage.reading);

  // test with min readable pos data
  s_test_mode = ADS1259_MODE_MIN_POS_DATA;
  ads1259_get_conversion_data(&s_storage);
  delay_ms(TEST_DATA_SETTLING_TIME_MS);
  TEST_ASSERT_EQUAL(0.000095, s_storage.reading);

  // test with min readable pos data
  s_test_mode = ADS1259_MODE_MIN_NEG_DATA;
  ads1259_get_conversion_data(&s_storage);
  delay_ms(TEST_DATA_SETTLING_TIME_MS);
  TEST_ASSERT_EQUAL(-0.000095, s_storage.reading);

  // test with zero data
  s_test_mode = ADS1259_MODE_ZERO_DATA;
  ads1259_get_conversion_data(&s_storage);
  delay_ms(TEST_DATA_SETTLING_TIME_MS);
  TEST_ASSERT_EQUAL(0, s_storage.reading);

  // test with a random data set
  s_test_mode = ADS1259_MODE_MIXED_DATA;
  test_raw = 0x102030;
  ads1259_get_conversion_data(&s_storage);
  delay_ms(TEST_DATA_SETTLING_TIME_MS);
  TEST_ASSERT_EQUAL(0x10, s_storage.conv_data.MSB);
  TEST_ASSERT_EQUAL(0x20, s_storage.conv_data.MID);
  TEST_ASSERT_EQUAL(0x30, s_storage.conv_data.LSB);
  TEST_ASSERT_EQUAL(test_raw, s_storage.conv_data.raw);
  TEST_ASSERT_EQUAL(6.299126, s_storage.reading);

  // test checksum fault triggered
  s_test_mode = ADS1259_MODE_CHECKSUM_FAULT;
  s_checksum = false;
  ads1259_get_conversion_data(&s_storage);
  delay_ms(TEST_DATA_SETTLING_TIME_MS);
  TEST_ASSERT_TRUE(s_checksum);

  // test out of range flag triggered
  s_test_mode = ADS1259_MODE_OUT_RANGE_FLAG_TRIGGERED;
  s_out_of_range = false;
  ads1259_get_conversion_data(&s_storage);
  delay_ms(TEST_DATA_SETTLING_TIME_MS);
  TEST_ASSERT_TRUE(s_out_of_range);
}
