#include "log.h"
#include "spv1020_mppt.h"
#include "spv1020_mppt_defs.h"
#include "test_helpers.h"
#include "unity.h"

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

void setup_test() {
  gpio_init();
  SpiSettings spi_settings = {
    .baudrate = 60000,
    .mode = SPI_MODE_3,
    .mosi = TEST_MOSI_PIN,
    .miso = TEST_MISO_PIN,
    .sclk = TEST_SCLK_PIN,
    .cs = TEST_CS_PIN,
  };
  spi_init(TEST_SPI_PORT, &spi_settings);
}
void teardown_test() {}

// Test that we can successfully send the SHUT and Turn ON commands.
void test_shut_and_turn_on(void) {
  TEST_ASSERT_OK(spv1020_shut(TEST_SPI_PORT));
  TEST_ASSERT_OK(spv1020_turn_on(TEST_SPI_PORT));
  TEST_ASSERT_OK(spv1020_shut(TEST_SPI_PORT));
  TEST_ASSERT_OK(spv1020_shut(TEST_SPI_PORT));  // 2 in a row, there should be a warning on x86
  TEST_ASSERT_OK(spv1020_turn_on(TEST_SPI_PORT));
  TEST_ASSERT_OK(spv1020_turn_on(TEST_SPI_PORT));  // unnecessary, there should be a warning on x86
}

// Test that we can successfully read a current and it's in the stated range.
void test_read_current(void) {
  uint16_t current = 0xFFFF;  // over 10 bits
  TEST_ASSERT_OK(spv1020_read_current(TEST_SPI_PORT, &current));

  // make sure the 6 highest bits are low, so it's a 10-bit value
  TEST_ASSERT_BITS_LOW(0b1111110000000000, current);
}

// Test that we can successfully read a voltage and it's in the stated range.
void test_read_voltage_in(void) {
  uint16_t vin = 0xFFFF;  // over 10 bits
  TEST_ASSERT_OK(spv1020_read_voltage_in(TEST_SPI_PORT, &vin));

  // make sure the 6 highest bits are low, so it's a 10-bit value
  TEST_ASSERT_BITS_LOW(0b1111110000000000, vin);
}

// Test that we can successfully read PWM and it's in the stated range.
void test_read_pwm(void) {
  uint16_t pwm = 0xFFFF;  // over 900
  TEST_ASSERT_OK(spv1020_read_pwm(TEST_SPI_PORT, &pwm));

  // make sure it's in [50, 900] and is even (accurate to 0.2%)
  TEST_ASSERT_TRUE_MESSAGE(pwm >= 50 && pwm <= 900, "PWM out of range [50, 900]");
  TEST_ASSERT_TRUE_MESSAGE(pwm % 2 == 0, "PWM not even (not accurate to 0.2%)");
}

// Test that we can successfully read a status and it's only 7 bits.
void test_read_status(void) {
  uint8_t status = 0xFF;  // 8th bit is set
  TEST_ASSERT_OK(spv1020_read_status(TEST_SPI_PORT, &status));

  // make sure the highest bit is low, so it's a 7-bit value
  TEST_ASSERT_BIT_LOW(7, status);

  if (status != 0) {
    // if we're testing on hardware we should know if it's an error
    LOG_WARN("SPV1020 status is nonzero: %x\r\n", status);
  }
}
