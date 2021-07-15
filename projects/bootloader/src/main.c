#include "bootloader_can.h"
#include "can_datagram.h"
#include "config.h"
#include "crc32.h"
#include "dispatcher.h"
#include "flash.h"
#include "interrupt.h"
#include "jump_to_application.h"
#include "log.h"
#include "ping.h"
#include "soft_timer.h"

typedef enum {
  CAN_DATAGRAM_EVENT_RX = 0,
  CAN_DATAGRAM_EVENT_TX,
  CAN_DATAGRAM_EVENT_FAULT,
  NUM_CAN_DATAGRAM_EVENTS,  // 3
} CanDatagramCanEvent;

typedef enum {
  DATAGRAM_EVENT_TX = NUM_CAN_DATAGRAM_EVENTS,  // 3
  DATAGRAM_EVENT_RX,
  DATAGRAM_EVENT_REPEAT,
  DATAGRAM_EVENT_ERROR,
  NUM_DATAGRAM_DIGEST_EVENTS,
} CanDatagramEvent;

static CanStorage s_can_storage;
static CanSettings can_settings = {
  .loopback = true,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = CAN_DATAGRAM_EVENT_RX,
  .tx_event = CAN_DATAGRAM_EVENT_TX,
  .fault_event = CAN_DATAGRAM_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
};

static CanDatagramSettings datagram_settings = {
  .tx_event = DATAGRAM_EVENT_TX,
  .rx_event = DATAGRAM_EVENT_RX,
  .repeat_event = DATAGRAM_EVENT_REPEAT,
  .error_event = DATAGRAM_EVENT_ERROR,
  .error_cb = NULL,
};

int main(void) {
  LOG_DEBUG("Hello from the bootloader!\n");
  // initialize all the modules
  // can and datagram
  bootloader_can_init(&s_can_storage, &can_settings);
  can_datagram_init(&datagram_settings);

  dispatcher_init();
  flash_init();
  interrupt_init();
  soft_timer_init();
  crc32_init();
  config_init();
  ping_init();

  jump_to_application();
  // not reached

  // there should be an event loop in main here,
  // jump_to_application will only be called when the command is issued
  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
    }
    can_process_event(&e);
    can_datagram_process_event(&e);
    // I forgot how to do this :( event loop
  }

  return 0;
}
