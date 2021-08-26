#pragma once
/* Test helpers for datagrams
 */

#include "bootloader_can.h"
#include "can_datagram.h"
#include "status.h"

#define DATAGRAM_PROCESS_ALL(e)                                 \
  while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) { \
    MS_TEST_HELPER_AWAIT_EVENT(e);                              \
    can_datagram_process_event(&e);                             \
    can_process_event(&e);                                      \
  }

// initialize the datagram helper and dependencies
void init_datagram_helper(CanStorage *can_storage, CanSettings *can_settings, uint8_t board_id,
                          CanDatagramSettings *can_datagram_settings);

// mock send the datagram |tx_config| from the client script, with datagram id of 0
// this function only process events until all can messages are sent
// which may leave some datagram rx events unprocessed,
// use DATAGRAM_PROCESS_ALL to finish processing all events
StatusCode mock_tx_datagram(CanDatagramTxConfig *tx_config);

// mock recieve a datagram to |rx_config| as the client script
// recieves the next datagram sent through can
StatusCode mock_rx_datagram(CanDatagramRxConfig *rx_config);
