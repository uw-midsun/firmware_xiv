#include "bootloader_can.h"
#include "bootloader_events.h"
#include "can_datagram.h"
#include "config.h"
#include "crc32.h"
#include "dispatcher.h"
#include "event_queue.h"
#include "flash.h"
#include "interrupt.h"
#include "jump_to_application.h"
#include "log.h"
#include "ping.h"
#include "soft_timer.h"
#include "wait.h"

static CanStorage s_can_storage;
static CanSettings s_can_settings = {
  .loopback = false,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = CAN_DATAGRAM_EVENT_RX,
  .tx_event = CAN_DATAGRAM_EVENT_TX,
  .fault_event = CAN_DATAGRAM_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
};

static CanDatagramSettings s_datagram_settings = {
  .tx_event = DATAGRAM_EVENT_TX,
  .rx_event = DATAGRAM_EVENT_RX,
  .repeat_event = DATAGRAM_EVENT_REPEAT,
  .error_event = DATAGRAM_EVENT_ERROR,
  .error_cb = NULL,
};

int main(void) {
  LOG_DEBUG("Hello from the bootloader!\n");

  flash_init();
  interrupt_init();
  soft_timer_init();
  crc32_init();
  event_queue_init();
  config_init();

  // gets the board id
  BootloaderConfig blconfig = {
    .controller_board_id = 0xFF,
  };
  config_get(&blconfig);
  uint8_t board_id = blconfig.controller_board_id;

  // can and datagram
  bootloader_can_init(&s_can_storage, &s_can_settings, board_id);
  can_datagram_init(&s_datagram_settings);

  dispatcher_init(board_id);
  ping_init(board_id);

  jump_to_application();
  // not reached

  // there should be an event loop in main here,
  // jump_to_application will only be called when the command is issued
  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
      can_datagram_process_event(&e);
    }
    wait();
  }

  return 0;
}
