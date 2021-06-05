#include "can_104.h"
#include "isc_104.h"
#include "spi_104.h"

#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define CAN_DEVICE_ID 0x1

typedef enum {
  CAN_EVENT_RX = 0,
  CAN_EVENT_TX,
  CAN_EVENT_FAULT,
} CanEvent;

static CanStorage can_store;

int main() {
  // Initializations
  gpio_init();
  interrupt_init();
  event_queue_init();
  soft_timer_init();

  // CAN set up
  CanSettings can_settings = {
    .device_id = CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_1000KBPS,
    .rx_event = CAN_EVENT_RX,
    .tx_event = CAN_EVENT_TX,
    .fault_event = CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 2 },  // Just using random pins since it hasn't been specified
    .rx = { GPIO_PORT_A, 3 },
    .loopback = false
  };
  can_init(&can_store, &can_settings);
  can_register_rx_default_handler(CAN_callback, NULL);

  LOG_DEBUG("Writing CAN messages\n");

  // CAN command
  write_CAN_messages();

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
  }
  // I2C command
  // send_i2c_message();
  // SPI command
  // send_spi_message();

  return 0;
}
