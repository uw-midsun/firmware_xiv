#include "bootloader_crc32.h"
#include "jump_to_application_dispatcher.h"

#include "bootloader_datagram_defs.h"
#include "bootloader_events.h"
#include "bootloader_mcu.h"
#include "config.h"
#include "crc32.h"
#include "dispatcher.h"
#include "flash.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helper_datagram.h"
#include "persist.h"
#include "test_helpers.h"
#include "unity.h"

#define FAILURE_STATUS 1
#define SUCCESS_STATUS 0

#define BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE (FLASH_ADDR_TO_PAGE(BOOTLOADER_CONFIG_PAGE_1_START))
#define BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE (FLASH_ADDR_TO_PAGE(BOOTLOADER_CONFIG_PAGE_2_START))

static uint8_t s_rx_data;
static uint8_t s_board_id = 2;

static BootloaderConfig test_input_config = { .crc32 = 1,
                                              .controller_board_id = 2,
                                              .controller_board_name = "a",
                                              .project_present = true,
                                              .project_name = "a",
                                              .project_info = "a",
                                              .git_version = "a",
                                              .application_crc32 = 1,
                                              .application_size = 1 };

static CanStorage s_test_can_storage;
static CanSettings s_test_can_settings = {
  .loopback = true,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = CAN_DATAGRAM_EVENT_RX,
  .tx_event = CAN_DATAGRAM_EVENT_TX,
  .fault_event = CAN_DATAGRAM_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
};

static CanDatagramSettings s_test_datagram_settings = {
  .tx_event = DATAGRAM_EVENT_TX,
  .rx_event = DATAGRAM_EVENT_RX,
  .repeat_event = DATAGRAM_EVENT_REPEAT,
  .error_event = DATAGRAM_EVENT_ERROR,
  .error_cb = NULL,
};

// intialize memory with random numbers for testing
StatusCode initialize_memory(uint8_t invalid) {
  size_t curr_size = 0;
  // writing the memory using a buffer with 2048 bytes at a time
  uint8_t buffer[2048];

  if (invalid) {
    // setting everything to 1 for testing purposes
    for (uint16_t i = 0; i < 2048; i++) {
      buffer[i] = 1;
    }
  } else {
    // generating random values in range of uint8(0->255)*rand_factor
    for (uint16_t i = 0; i < 2048; i++) {
      buffer[i] = i % 231;
    }
  }

  while (curr_size < BOOTLOADER_APPLICATION_SIZE) {
    // write flash
    flash_write((uintptr_t)BOOTLOADER_APPLICATION_START + curr_size, buffer, sizeof(buffer));
    curr_size += sizeof(buffer);
  }

  return STATUS_CODE_OK;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  gpio_init();
  soft_timer_init();
  flash_init();
  crc32_init();

  flash_erase(BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE);
  flash_erase(BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE);
  config_init();

  ms_test_helper_datagram_init(&s_test_can_storage, &s_test_can_settings, s_board_id,
                               &s_test_datagram_settings);
  dispatcher_init(s_board_id);
}

void teardown_test(void) {}

void test_jump_to_application_works(void) {
  // to intialize memory we erase all
  // the flash pages and intialize it after
  for (int i = 0; i < NUM_FLASH_PAGES; i++) {
    flash_erase((FlashPage)i);
  }
  TEST_ASSERT_OK(initialize_memory(0));

  // commit the config so we have the updated application crc32 code
  TEST_ASSERT_OK(config_commit(&test_input_config));

  TEST_ASSERT_OK(jump_to_application_dispatcher_init());
  CanDatagramTxConfig tx_config = {
    .dgram_type = BOOTLOADER_DATAGRAM_STATUS_RESPONSE,
    .destination_nodes_len = 1,
    .destination_nodes = &s_board_id,
    .data_len = 0,
    .data = NULL,
    .tx_cmpl_cb = tx_cmpl_cb,
  };
  CanDatagramRxConfig rx_config = {
    .destination_nodes = NULL,
    .data_len = 1,
    .data = &s_rx_data,
    .node_id = 0,
    .rx_cmpl_cb = NULL,
  };

  dgram_helper_mock_tx_datagram(&tx_config);
  dgram_helper_mock_rx_datagram(&rx_config);

  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
  TEST_ASSERT_EQUAL(SUCCESS_STATUS, *(rx_config.data));
  TEST_ASSERT_EQUAL(BOOTLOADER_DATAGRAM_STATUS_RESPONSE, rx_config.dgram_type);
}

// to be developed once flash bug is fixed
void test_jump_to_application_failure(void) {
  // to intialize memory we erase all
  // the flash pages and intialize it after
  for (int i = 0; i < NUM_FLASH_PAGES; i++) {
    flash_erase((FlashPage)i);
  }
  TEST_ASSERT_OK(initialize_memory(0));

  // commit the config so we have the updated application crc32 code
  TEST_ASSERT_OK(config_commit(&test_input_config));

  // now we erase the flash pages and update the memory
  for (int i = 0; i < NUM_FLASH_PAGES; i++) {
    flash_erase((FlashPage)i);
  }
  TEST_ASSERT_OK(initialize_memory(1));

  TEST_ASSERT_OK(jump_to_application_dispatcher_init());
  CanDatagramTxConfig tx_config = {
    .dgram_type = BOOTLOADER_DATAGRAM_STATUS_RESPONSE,
    .destination_nodes_len = 1,
    .destination_nodes = &s_board_id,
    .data_len = 0,
    .data = NULL,
    .tx_cmpl_cb = tx_cmpl_cb,
  };
  CanDatagramRxConfig rx_config = {
    .destination_nodes = NULL,
    .data_len = 1,
    .data = &s_rx_data,
    .node_id = 0,
    .rx_cmpl_cb = NULL,
  };

  dgram_helper_mock_tx_datagram(&tx_config);
  dgram_helper_mock_rx_datagram(&rx_config);

  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
  TEST_ASSERT_EQUAL(FAILURE_STATUS, *(rx_config.data));
  TEST_ASSERT_EQUAL(BOOTLOADER_DATAGRAM_STATUS_RESPONSE, rx_config.dgram_type);
}
