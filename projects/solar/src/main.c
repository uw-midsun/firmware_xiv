#include <stdbool.h>

#include "adc.h"
#include "can.h"
#include "command_rx.h"
#include "data_store.h"
#include "data_tx.h"
#include "drv120_relay.h"
#include "event_queue.h"
#include "fault_handler.h"
#include "fault_monitor.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "logger.h"
#include "mcp3427_adc.h"
#include "mppt.h"
#include "relay_fsm.h"
#include "sense.h"
#include "sense_mcp3427.h"
#include "sense_mppt.h"
#include "sense_temperature.h"
#include "soft_timer.h"
#include "solar_boards.h"
#include "solar_config.h"
#include "spi.h"
#include "status.h"
#include "wait.h"

// TODO(SOFT-293): get hardware to solder something so we can detect MPPT counts instead of this
#ifndef MPPT_COUNT
#define MPPT_COUNT SOLAR_BOARD_6_MPPTS
#endif

#define SENSE_CYCLE_PERIOD_US 1000000  // 1 second

static CanStorage s_can_storage;
static RelayFsmStorage s_relay_fsm_storage = { 0 };

static StatusCode prv_initialize_libraries(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  status_ok_or_return(gpio_init());
  adc_init(ADC_MODE_SINGLE);

  status_ok_or_return(i2c_init(I2C_PORT_1, config_get_i2c1_settings()));
  status_ok_or_return(i2c_init(I2C_PORT_2, config_get_i2c2_settings()));
  status_ok_or_return(spi_init(SOLAR_SPI_PORT, config_get_spi_settings()));
  status_ok_or_return(can_init(&s_can_storage, config_get_can_settings()));

  status_ok_or_return(drv120_relay_init(config_get_drv120_relay_pin()));

  return STATUS_CODE_OK;
}

static StatusCode prv_initialize_sense_modules(SolarMpptCount mppt_count) {
  status_ok_or_return(mppt_init());

  status_ok_or_return(sense_init(config_get_sense_settings()));
  status_ok_or_return(sense_mcp3427_init(config_get_sense_mcp3427_settings(mppt_count)));
  status_ok_or_return(sense_mppt_init(config_get_sense_mppt_settings(mppt_count)));
  status_ok_or_return(sense_temperature_init(config_get_sense_temperature_settings(mppt_count)));

  return STATUS_CODE_OK;
}

static StatusCode prv_initialize_action_modules(void) {
  status_ok_or_return(relay_fsm_init(&s_relay_fsm_storage));
  status_ok_or_return(fault_handler_init(config_get_fault_handler_settings()));
  status_ok_or_return(command_rx_init());
  return STATUS_CODE_OK;
}

static StatusCode prv_initialize_data_consumer_modules(SolarMpptCount mppt_count) {
  status_ok_or_return(logger_init(mppt_count));
  status_ok_or_return(data_tx_init(config_get_data_tx_settings()));
  status_ok_or_return(fault_monitor_init(config_get_fault_monitor_settings(mppt_count)));
  return STATUS_CODE_OK;
}

int main(void) {
  status_ok_or_return(prv_initialize_libraries());

  status_ok_or_return(data_store_init());
  status_ok_or_return(prv_initialize_action_modules());
  status_ok_or_return(prv_initialize_sense_modules(MPPT_COUNT));
  status_ok_or_return(prv_initialize_data_consumer_modules(MPPT_COUNT));

  LOG_DEBUG("Hello from solar, initialized with %d MPPTs\n", MPPT_COUNT);

  sense_mcp3427_start();
  sense_start();

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      relay_fsm_process_event(&s_relay_fsm_storage, &e);
      can_process_event(&e);
      mcp3427_process_event(&e);
      fault_monitor_process_event(&e);
      data_tx_process_event(&e);
      logger_process_event(&e);
    }
    wait();
  }

  return STATUS_CODE_OK;
}
