#include "jump_to_application_dispatcher.h"

#include <stdint.h>

#include "bootloader_crc32.h"

#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "can_datagram.h"
#include "config.h"
#include "dispatcher.h"
#include "log.h"

// response config datagram setup
static uint8_t s_board_id;
static CanDatagramTxConfig s_response_success_config = {
  .dgram_type = BOOTLOADER_DATAGRAM_JUMP_TO_APP,
  .destination_nodes_len = 0,
  .destination_nodes = NULL,
  .data_len = 1,
  .data = 1,
  .tx_cb = bootloader_can_transmit,
  .tx_cmpl_cb = tx_cmpl_cb,
};

static CanDatagramTxConfig s_response_failure_config = {
  .dgram_type = BOOTLOADER_DATAGRAM_JUMP_TO_APP,
  .destination_nodes_len = 0,
  .destination_nodes = NULL,
  .data_len = 1,
  .data = 0,
  .tx_cb = bootloader_can_transmit,
  .tx_cmpl_cb = tx_cmpl_cb,
};

static StatusCode prv_jump_to_application_response(uint8_t *data, uint16_t data_len,
                                                   void *context) {
  BootloaderConfig config = { 0 };
  config_get(&config);

  // get the computed crc32 code
  uint32_t computed_crc32 = calculate_application_crc32((uintptr_t)BOOTLOADER_APPLICATION_START,
                                                        BOOTLOADER_APPLICATION_SIZE);
  if (config.application_crc32 != computed_crc32) {
    LOG_DEBUG("CRC32 codes do not match returning\n");
    return can_datagram_start_tx(&s_response_failure_config);
  }

  return can_datagram_start_tx(&s_response_success_config);
}

StatusCode jump_to_application_dispatcher_init(uint8_t board_id) {
  s_board_id = board_id;
  return dispatcher_register_callback(BOOTLOADER_DATAGRAM_STATUS_RESPONSE,
                                      prv_jump_to_application_response, NULL);
}
