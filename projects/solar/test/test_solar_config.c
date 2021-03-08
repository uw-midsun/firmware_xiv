#include "solar_config.h"

#include "adc.h"
#include "can.h"
#include "can_msg_defs.h"
#include "data_store.h"
#include "data_tx.h"
#include "drv120_relay.h"
#include "fault_monitor.h"
#include "i2c.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "relay_fsm.h"
#include "sense.h"
#include "sense_mcp3427.h"
#include "sense_mppt.h"
#include "sense_temperature.h"
#include "solar_events.h"
#include "spi.h"
#include "test_helpers.h"
#include "unity.h"

#define INVALID_MPPT_COUNT (MAX_SOLAR_BOARD_MPPTS + 1)

static CanStorage s_can_storage;

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_SOLAR_6_MPPTS, SOLAR_CAN_EVENT_TX,
                                  SOLAR_CAN_EVENT_RX, SOLAR_CAN_EVENT_FAULT);
  data_store_init();
  adc_init(ADC_MODE_SINGLE);

  SenseSettings sense_settings = { .sense_period_us = 100000 };
  sense_init(&sense_settings);
}
void teardown_test(void) {}

// Test that we can initialize with the I2C, SPI, and CAN settings we return.
void test_initializing_library_config(void) {
  TEST_ASSERT_OK(i2c_init(I2C_PORT_1, config_get_i2c1_settings()));
  TEST_ASSERT_OK(i2c_init(I2C_PORT_2, config_get_i2c2_settings()));
  TEST_ASSERT_OK(spi_init(SPI_PORT_2, config_get_spi_settings()));
  TEST_ASSERT_OK(can_init(&s_can_storage, config_get_can_settings(SOLAR_BOARD_6_MPPTS)));
}

// Test that we can initialize the config returned by |config_get_sense_settings|
void test_initializing_sense_config(void) {
  TEST_ASSERT_OK(sense_init(config_get_sense_settings()));
}

// Test that we can initialize the configs returned by |config_get_sense_temperature_settings|.
// This is separated into two tests to avoid exhausting the sense callbacks.
void test_initializing_sense_temperature_config_5_mppts(void) {
  TEST_ASSERT_OK(
      sense_temperature_init(config_get_sense_temperature_settings(SOLAR_BOARD_5_MPPTS)));
}
void test_initializing_sense_temperature_config_6_mppts(void) {
  TEST_ASSERT_OK(
      sense_temperature_init(config_get_sense_temperature_settings(SOLAR_BOARD_6_MPPTS)));
}

// Test that we can initialize the config returned by |config_get_sense_mcp3427_settings|.
// Again separate to avoid exhausting callbacks.
void test_initializing_sense_mcp3427_config_5_mppts(void) {
  TEST_ASSERT_OK(sense_mcp3427_init(config_get_sense_mcp3427_settings(SOLAR_BOARD_5_MPPTS)));
}
void test_initializing_sense_mcp3427_config_6_mppts(void) {
  TEST_ASSERT_OK(sense_mcp3427_init(config_get_sense_mcp3427_settings(SOLAR_BOARD_6_MPPTS)));
}

// Test that we can initialize the config returned by |config_get_sense_mppt_settings|.
// Again separate to avoid exhausting callbacks.
void test_initializing_sense_mppt_config_5_mppts(void) {
  TEST_ASSERT_OK(sense_mppt_init(config_get_sense_mppt_settings(SOLAR_BOARD_5_MPPTS)));
}
void test_initializing_sense_mppt_config_6_mppts(void) {
  TEST_ASSERT_OK(sense_mppt_init(config_get_sense_mppt_settings(SOLAR_BOARD_6_MPPTS)));
}

// Test that we can initialize the configs returned by |config_get_fault_monitor_settings|.
void test_initializing_fault_monitor_config(void) {
  TEST_ASSERT_OK(fault_monitor_init(config_get_fault_monitor_settings(SOLAR_BOARD_5_MPPTS)));
  TEST_ASSERT_OK(fault_monitor_init(config_get_fault_monitor_settings(SOLAR_BOARD_6_MPPTS)));
}

// Test that we can initialize the config returned by |config_get_fault_handler_settings|.
void test_initializing_fault_handler_config(void) {
  TEST_ASSERT_OK(fault_handler_init(config_get_fault_handler_settings()));
}

// Test that we can initialize the config returned by |config_get_fault_handler_settings|.
void test_initializing_data_tx_config(void) {
  TEST_ASSERT_OK(data_tx_init(config_get_data_tx_settings()));
}

// Test that passing an invalid MPPT count returns NULL.
void test_invalid_args(void) {
  TEST_ASSERT_NULL(config_get_sense_mcp3427_settings(INVALID_MPPT_COUNT));
  TEST_ASSERT_NULL(config_get_sense_temperature_settings(INVALID_MPPT_COUNT));
  TEST_ASSERT_NULL(config_get_sense_mppt_settings(INVALID_MPPT_COUNT));
  TEST_ASSERT_NULL(config_get_fault_monitor_settings(INVALID_MPPT_COUNT));
}
