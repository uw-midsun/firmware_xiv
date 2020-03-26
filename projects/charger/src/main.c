
// Charger Modules
// - charger_controller
//     - implements api for init, activate, and deactivate using mcp215
//     - charger_controller_fault_monitor (merged with charger controller)
//     - register can rx callbacks to find faults, then broadcasts them. implements init
// - [DONE] charger_control_pilot_monitor
//     - handles pwm reading requests, then raises an event with the result
//     - implements event_handler to be called from main
// - [DONE] charger_connection_sense
//     - handles polling connection sense pin, raises events to indicate state changes
//     - implements init
// - permission_resolver
//     - handles connection events
// - [TODOS] begin_charge_fsm
// - [TODOS] stop_charge_fsm
// - battery_voltage_monitor
//     - handles overvoltage faults from BMS

#include "charger_events.h"
#include "begin_charge_fsm.h"
#include "charger_connection_sense.h"
#include "charger_control_pilot_monitor.h"
#include "stop_charger.h"
#include "charger_controller.h"

#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "soft_timer.h"
#include "status.h"
#include "can.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
#include "generic_can_mcp2515.h"

#define TEST_CAN_DEVICE_ID 0x1

static CanStorage s_can_storage;
static GenericCanMcp2515 s_generic_can = { 0 };

static CanSettings s_can_settings = {
  .device_id = TEST_CAN_DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_250KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .rx_event = CHARGER_CAN_EVENT_RX,
  .tx_event = CHARGER_CAN_EVENT_TX,
  .fault_event = CHARGER_CAN_EVENT_FAULT,
  .loopback = false,
};

static Mcp2515Settings mcp2515_settings = {
  .spi_port = SPI_PORT_2,
  .spi_baudrate = 6000000,
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },
  .int_pin = { .port = GPIO_PORT_A, .pin = 8 },
  .can_bitrate = MCP2515_BITRATE_250KBPS,
  .loopback = false,
};

int main(void) {
  LOG_DEBUG("Welcome to Charger!\n");
  event_queue_init();
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();

  can_init(&s_can_storage, &s_can_settings);
  generic_can_mcp2515_init(&s_generic_can, &mcp2515_settings);

  charger_controller_init(&s_generic_can.base);
  connection_sense_init();
  control_pilot_monitor_init();
  begin_charge_fsm_init();

  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
      can_process_event(&e);
      control_pilot_monitor_process_event(&e);
      begin_fsm_process_event(&e);
      stop_charger_process_event(&e);
    }
  }
  return 0;
}
