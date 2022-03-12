#include "jump_to_application_dispatcher.h"

#include <inttypes.h>
#include <stdint.h>

#include "bootloader_crc32.h"
#include "jump_to_application.h"

#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "can_datagram.h"
#include "config.h"
#include "dispatcher.h"
#include "log.h"

#define FAILURE_STATUS 1
#define SUCCESS_STATUS 0

// response config datagram setup
static uint8_t s_status;

void prv_jump_to_application_tx_cmpl_cb() {
  jump_to_application();
}

static CanDatagramTxConfig s_response_config = {
  .dgram_type = BOOTLOADER_DATAGRAM_STATUS_RESPONSE,
  .destination_nodes_len = 0,
  .destination_nodes = NULL,
  .data_len = 1,
  .data = &s_status,
  .tx_cb = bootloader_can_transmit,
  .tx_cmpl_cb = prv_jump_to_application_tx_cmpl_cb,
};

static StatusCode prv_jump_to_application_response(uint8_t *data, uint16_t data_len,
                                                   void *context) {
  BootloaderConfig config = { 0 };
  config_get(&config);

  // get the computed crc32 code
  uint32_t computed_crc32 = calculate_application_crc32();
  LOG_DEBUG("application crc32 is " PRIu32 " vs computed crc32 " PRIu32 "\n\n",
            config.application_crc32, computed_crc32);

  if (config.application_crc32 != computed_crc32) {
    LOG_DEBUG("CRC32 codes do not match!!!\n");
    s_status = FAILURE_STATUS;
    return can_datagram_start_tx(&s_response_config);
  }

  s_status = SUCCESS_STATUS;
  return can_datagram_start_tx(&s_response_config);
}

StatusCode jump_to_application_dispatcher_init() {
  return dispatcher_register_callback(BOOTLOADER_DATAGRAM_STATUS_RESPONSE,
                                      prv_jump_to_application_response, NULL);
}
