#include <string.h>

#include "ads1259_adc.h"
#include "bms.h"
#include "current_sense.h"
#include "delay.h"
#include "exported_enums.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "test_helpers.h"

#define TEST_CS_CONV_DELAY 10

static SpiSettings s_spi_settings = {
  .baudrate = 6000000,
  .mosi = { .port = GPIO_PORT_B, 15 },
  .miso = { .port = GPIO_PORT_B, 14 },
  .sclk = { .port = GPIO_PORT_B, 13 },
  .cs = { .port = GPIO_PORT_B, 12 },
};

static CurrentReadings s_reads;
static uint8_t s_fault_bps_bitmask = 0;
static bool s_fault_bps_clear = false;

static double s_ads_read = 0.0;
static Ads1259ErrorHandlerCb s_ads_cb = NULL;

StatusCode TEST_MOCK(fault_bps)(uint8_t fault_bitmask, bool clear) {
  s_fault_bps_bitmask = fault_bitmask;
  s_fault_bps_clear = clear;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(ads1259_get_conversion_data)(Ads1259Storage *storage) {
  storage->reading = s_ads_read;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(ads1259_init)(Ads1259Storage *storage, Ads1259Settings *settings) {
  s_ads_cb = settings->handler;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  memset(&s_reads, 0, sizeof(s_reads));
  s_fault_bps_bitmask = 0;
  s_fault_bps_clear = false;

  s_ads_read = 0.0;
  s_ads_cb = NULL;
}

void teardown_test(void) {}

void print_reads() {
  for (int i = 0; i < NUM_STORED_CURRENT_READINGS; i++) {
    printf("%d: %d\n", i, s_reads.readings[i]);
  }
  printf("avg: %d\n", s_reads.average);
}

void test_is_charging(void) {
  s_ads_read = 0.5;
  TEST_ASSERT_OK(current_sense_init(&s_reads, &s_spi_settings, TEST_CS_CONV_DELAY));
  delay_ms(TEST_CS_CONV_DELAY * NUM_STORED_CURRENT_READINGS);
  TEST_ASSERT_FALSE(current_sense_is_charging());
  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_CURRENT_SENSE, s_fault_bps_bitmask);
  TEST_ASSERT(s_fault_bps_clear);

  s_ads_read = -0.5;
  delay_ms(TEST_CS_CONV_DELAY * NUM_STORED_CURRENT_READINGS);
  TEST_ASSERT_TRUE(current_sense_is_charging());
  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_CURRENT_SENSE, s_fault_bps_bitmask);
  TEST_ASSERT(s_fault_bps_clear);
}

void test_oc_discharging(void) {
  s_ads_read = 1.5;
  TEST_ASSERT_OK(current_sense_init(&s_reads, &s_spi_settings, TEST_CS_CONV_DELAY));
  delay_ms(TEST_CS_CONV_DELAY * NUM_STORED_CURRENT_READINGS);
  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_CURRENT_SENSE, s_fault_bps_bitmask);
  TEST_ASSERT_FALSE(s_fault_bps_clear);
}

void test_oc_charging(void) {
  s_ads_read = -0.9;
  s_fault_bps_clear = true;
  TEST_ASSERT_OK(current_sense_init(&s_reads, &s_spi_settings, TEST_CS_CONV_DELAY));
  delay_ms(TEST_CS_CONV_DELAY * NUM_STORED_CURRENT_READINGS);
  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_CURRENT_SENSE, s_fault_bps_bitmask);
  TEST_ASSERT_FALSE(s_fault_bps_clear);
}

void test_ring_no_segfault(void) {
  s_ads_read = 0.03;
  TEST_ASSERT_OK(current_sense_init(&s_reads, &s_spi_settings, TEST_CS_CONV_DELAY));
  delay_ms(TEST_CS_CONV_DELAY * 5);
  s_ads_read = 0.01;
  delay_ms(TEST_CS_CONV_DELAY * (NUM_STORED_CURRENT_READINGS + 3));  // would segfault if incorrect
  // assert it's the calculated value based on 0.01, not 0.03
  TEST_ASSERT_EQUAL(100, s_reads.average);
}

void test_error_cb(void) {
  s_fault_bps_clear = true;
  TEST_ASSERT_OK(current_sense_init(&s_reads, &s_spi_settings, TEST_CS_CONV_DELAY));
  s_ads_cb(ADS1259_STATUS_CODE_CHECKSUM_FAULT, NULL);
  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_CURRENT_SENSE, s_fault_bps_bitmask);
  TEST_ASSERT_FALSE(s_fault_bps_clear);
}
