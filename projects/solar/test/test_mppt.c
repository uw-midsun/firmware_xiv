#include "log.h"
#include "mppt.h"
#include "mux.h"
#include "spv1020_mppt.h"
#include "spv1020_mppt_defs.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_SPI_PORT SPI_PORT_2

static uint8_t s_times_mux_set_called = 0;
StatusCode TEST_MOCK(mux_set)(MuxAddress *address, uint8_t selected) {
  if (selected >= (1 << address->bit_width)) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  ++s_times_mux_set_called;

  return STATUS_CODE_OK;
}

void setup_test() {
  gpio_init();
  SpiSettings spi_settings = { .baudrate = 60000,
                               .mode = SPI_MODE_3,
                               .mosi = { .port = GPIO_PORT_B, 15 },
                               .miso = { .port = GPIO_PORT_B, 14 },
                               .sclk = { .port = GPIO_PORT_B, 13 },
                               .cs = { .port = GPIO_PORT_B, 12 } };
  spi_init(TEST_SPI_PORT, &spi_settings);
}

void teardown_test() {}

// Test that we can successfully send the SHUT and Turn ON commands.
void test_shut_and_turn_on(void) {
  s_times_mux_set_called = 0;
  TEST_ASSERT_OK(mppt_shut(TEST_SPI_PORT, (uint8_t)0b0001));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 2);
  TEST_ASSERT_OK(mppt_turn_on(TEST_SPI_PORT, (uint8_t)0b0001));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 4);
  TEST_ASSERT_OK(mppt_shut(TEST_SPI_PORT, (uint8_t)0b0001));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 6);
  TEST_ASSERT_OK(
      mppt_shut(TEST_SPI_PORT, (uint8_t)0b0001));  // 2 in a row, there should be a warning on x86
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 8);
  TEST_ASSERT_OK(mppt_turn_on(TEST_SPI_PORT, (uint8_t)0b0001));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 10);
  TEST_ASSERT_OK(mppt_turn_on(TEST_SPI_PORT,
                              (uint8_t)0b0001));  // unnecessary, there should be a warning on x86
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 12);

  // For an out of range selected value
  TEST_ASSERT_NOT_OK(mppt_turn_on(TEST_SPI_PORT, (uint8_t)0b1110));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 12);
  TEST_ASSERT_NOT_OK(mppt_turn_on(TEST_SPI_PORT, (uint8_t)0b1010));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 12);
  TEST_ASSERT_OK(mppt_turn_on(TEST_SPI_PORT, (uint8_t)0b0110));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 14);
}

// Test that we can successfully read a current and it's in the stated range.
void test_read_current(void) {
  s_times_mux_set_called = 0;
  uint16_t current = 0xFFFF;  // over 10 bits
  TEST_ASSERT_NOT_OK(mppt_read_current(TEST_SPI_PORT, &current, (uint8_t)0b1001));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 0);
  TEST_ASSERT_OK(mppt_read_current(TEST_SPI_PORT, &current, (uint8_t)0b0001));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 2);

  // make sure the 6 highest bits are low, so it's a 10-bit value
  TEST_ASSERT_BITS_LOW(0b1111110000000000, current);
}

// Test that we can successfully read a voltage and it's in the stated range.
void test_read_voltage_in(void) {
  s_times_mux_set_called = 0;
  uint16_t vin = 0xFFFF;  // over 10 bits
  TEST_ASSERT_NOT_OK(mppt_read_voltage_in(TEST_SPI_PORT, &vin, (uint8_t)0b1101));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 0);
  TEST_ASSERT_OK(mppt_read_voltage_in(TEST_SPI_PORT, &vin, (uint8_t)0b0001));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 2);

  // make sure the 6 highest bits are low, so it's a 10-bit value
  TEST_ASSERT_BITS_LOW(0b1111110000000000, vin);
}

// Test that we can successfully read PWM and it's in the stated range.
void test_read_pwm(void) {
  s_times_mux_set_called = 0;
  uint16_t pwm = 0xFFFF;  // over 900
  TEST_ASSERT_NOT_OK(mppt_read_pwm(TEST_SPI_PORT, &pwm, (uint8_t)0b110001));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 0);
  TEST_ASSERT_OK(mppt_read_pwm(TEST_SPI_PORT, &pwm, (uint8_t)0b0001));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 2);

  // make sure it's in [50, 900] and is even (accurate to 0.2%)
  TEST_ASSERT_TRUE_MESSAGE(pwm >= 50 && pwm <= 900, "PWM out of range [50, 900]");
  TEST_ASSERT_TRUE_MESSAGE(pwm % 2 == 0, "PWM not even (not accurate to 0.2%)");
}

// Test that we can successfully read a status and it's only 7 bits.
void test_read_status(void) {
  s_times_mux_set_called = 0;
  uint8_t status = 0xFF;  // 8th bit is set
  TEST_ASSERT_NOT_OK(mppt_read_status(TEST_SPI_PORT, &status, (uint8_t)0b010001));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 0);
  TEST_ASSERT_OK(mppt_read_status(TEST_SPI_PORT, &status, (uint8_t)0b0001));
  TEST_ASSERT_EQUAL(s_times_mux_set_called, 2);

  // make sure the highest bit is low, so it's a 7-bit value
  TEST_ASSERT_BIT_LOW(7, status);

  if (status != 0) {
    // if we're testing on hardware we should know if it's an error
    LOG_WARN("mppt status is nonzero: %x\r\n", status);
  }
}
