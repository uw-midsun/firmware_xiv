#pragma once
// datagram test helper
// Requires event queue, interrupt, soft timer, crc32, bootloader can, and datagram to be initialize

// as datagram will stay active if a rx_cmpl_cb starts another datagram tx,
// mock_tx_datagram without needing to rx any response should be followed by
// dgram_helper_process_all() to process the rest of the datagram rx events

#include "bootloader_can.h"
#include "can_datagram.h"
#include "ms_test_helpers.h"
#include "status.h"

// initialize the datagram helper
// initializes bootloader can and can datagram.
StatusCode ms_test_helper_datagram_init(CanStorage *can_storage, CanSettings *can_settings,
                                        uint8_t board_id,
                                        CanDatagramSettings *can_datagram_settings);

// mock send the datagram |tx_config| from the client script, with datagram id of 0
// this function only process events until all can messages are sent
// which may leave some datagram rx events unprocessed,
// use datagram_process_all to finish processing all events
StatusCode dgram_helper_mock_tx_datagram(CanDatagramTxConfig *tx_config);

// mock receive a datagram to |rx_config| as the client script
// recieves the next datagram sent through can
StatusCode dgram_helper_mock_rx_datagram(CanDatagramRxConfig *rx_config);

// process all events until datagram is not active
// required to complete datagram rx when mock_tx_datagram does not expect a response
void dgram_helper_process_all(void);
