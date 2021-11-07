#include "ms_test_helper_datagram.h"

#include "event_queue.h"
#include "fifo.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "string.h"

#define TEST_CLIENT_SCRIPT_ID 0
#define CAN_BUFFER_SIZE 8

static EventId s_can_tx_event;
static EventId s_dgram_error_event;

// rx and tx uses the same fifo, buffer, and cmpl_cb storage
// the fifo is not persistent across individual rx and tx helper calls
// they are global as required by prv_dgram_to_fifo and prv_dgram_to_fifo_cmpl_cb
static bool s_cmpl;
static CanDatagramExitCb s_cmpl_cb;
static Fifo s_dgram_data_fifo;
static uint8_t s_dgram_data_buffer[2060];  // 2058 is enough to fix a max sized datagram
static bool s_transmit_start_msg;

static StatusCode prv_dgram_to_fifo(uint8_t *data, size_t len, bool is_start_message) {
  // a start message reinitalize the fifo and changes the msg type
  if (is_start_message) {  // empty fifo
    return fifo_init(&s_dgram_data_fifo, s_dgram_data_buffer);
  }
  return fifo_push_arr(&s_dgram_data_fifo, data, len);
}

static StatusCode prv_fifo_to_dgram(BootloaderCanCallback transmitter) {
  if (s_transmit_start_msg) {
    s_transmit_start_msg = false;
    return transmitter(NULL, 0, true);
  }
  uint8_t data[CAN_BUFFER_SIZE];
  size_t len = MIN(fifo_size(&s_dgram_data_fifo), (size_t)CAN_BUFFER_SIZE);
  fifo_pop_arr(&s_dgram_data_fifo, data, len);
  return transmitter(data, len, false);
}

// callback when datagram is finished with tx or rx-ing a datagram
static void prv_dgram_to_fifo_cmpl_cb(void) {
  s_cmpl = true;
  if (s_cmpl_cb != NULL) s_cmpl_cb();
}

// sends a can message as a client script,
// effectively the same as bootloader_can_transmit with msg_id of 0.
static StatusCode prv_tx_as_client(uint8_t *data, size_t len, bool is_start_msg) {
  CanMessage message = {
    .source_id = SYSTEM_CAN_DEVICE_BOOTLOADER,
    .msg_id = TEST_CLIENT_SCRIPT_ID,
    .dlc = len,
    .type = is_start_msg ? CAN_MSG_TYPE_ACK : CAN_MSG_TYPE_DATA,
  };
  memcpy(message.data_u8, data, len);
  return can_transmit(&message, NULL);
}

// mock transmit a datagram from the client
StatusCode dgram_helper_mock_tx_datagram(CanDatagramTxConfig *tx_config) {
  tx_config->tx_cb = prv_dgram_to_fifo;
  s_cmpl = false;

  s_cmpl_cb = tx_config->tx_cmpl_cb;
  tx_config->tx_cmpl_cb = prv_dgram_to_fifo_cmpl_cb;
  status_ok_or_return(can_datagram_start_tx(tx_config));

  // send the datagram to fifo
  Event e = { 0 };
  while (!s_cmpl) {
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
  }
  // send messages from fifo to can
  s_transmit_start_msg = true;
  while (fifo_size(&s_dgram_data_fifo) > 0) {
    status_ok_or_return(prv_fifo_to_dgram(prv_tx_as_client));
    do {  // until another tx event is processed
      MS_TEST_HELPER_AWAIT_EVENT(e);
      can_datagram_process_event(&e);
      can_process_event(&e);
    } while (e.id != s_can_tx_event);
  }
  // set tx_cmpl_cb back
  tx_config->tx_cmpl_cb = s_cmpl_cb;
  return STATUS_CODE_OK;
}

// mock receive a datagram as the client
StatusCode dgram_helper_mock_rx_datagram(CanDatagramRxConfig *rx_config) {
  s_cmpl_cb = rx_config->rx_cmpl_cb;
  s_cmpl = false;
  rx_config->rx_cmpl_cb = prv_dgram_to_fifo_cmpl_cb;

  // process the response to fifo
  Event e = { 0 };
  status_ok_or_return(bootloader_can_register_debug_handler(prv_dgram_to_fifo));
  do {
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
    can_process_event(&e);
  } while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE);

  status_ok_or_return(can_datagram_start_listener(rx_config));
  // send data from fifo to can_datagram
  s_transmit_start_msg = true;
  while (!s_cmpl) {
    status_ok_or_return(prv_fifo_to_dgram(can_datagram_rx));
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
  }
  // set rx_cmpl_cb back
  rx_config->rx_cmpl_cb = s_cmpl_cb;
  return STATUS_CODE_OK;
}

StatusCode prv_no_response_helper(uint8_t *data, size_t len, bool is_start_msg) {
  TEST_ASSERT_FALSE_MESSAGE(is_start_msg, "Detected datagram response, expected none");
  return STATUS_CODE_OK;
}
// process all events until datagram is inactive
// make sure no more
void dgram_helper_assert_no_response(void) {
  // checks that no more datagram is started
  bootloader_can_register_debug_handler(prv_no_response_helper);
  Event e = { 0 };
  while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {
    MS_TEST_HELPER_AWAIT_EVENT(e);
    can_datagram_process_event(&e);
    can_process_event(&e);

    // exit the loop if there is a datagram error
    if (e.id == s_dgram_error_event) {
      break;
    }
  }
}

// initialize the datagram helper
StatusCode ms_test_helper_datagram_init(CanStorage *can_storage, CanSettings *can_settings,
                                        uint8_t board_id,
                                        CanDatagramSettings *can_datagram_settings) {
  status_ok_or_return(bootloader_can_init(can_storage, can_settings, board_id));
  status_ok_or_return(can_datagram_init(can_datagram_settings));

  s_can_tx_event = can_storage->tx_event;
  s_dgram_error_event = can_datagram_settings->error_event;
  return fifo_init(&s_dgram_data_fifo, s_dgram_data_buffer);
}
