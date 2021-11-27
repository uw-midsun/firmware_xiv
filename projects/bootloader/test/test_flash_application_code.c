#include "bootloader_datagram_defs.h"
#include "bootloader_events.h"
#include "bootloader_mcu.h"
#include "config.h"
#include "crc32.h"
#include "dispatcher.h"
#include "flash.h"
#include "flash_application_code.h"
#include "flash_application_code.pb.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helper_datagram.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "string.h"

#define MAX_STRING_SIZE 64
#define PROTOBUF_MAXSIZE 128

static uint8_t s_client_id = 0;
static uint8_t s_board_id = 2;

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

static uint8_t s_tx_data[DGRAM_MAX_DATA_SIZE];

static uint8_t s_destination_nodes[DGRAM_MAX_DEST_NODES_SIZE];
static uint8_t s_rx_data[PROTOBUF_MAXSIZE];
static uint16_t s_rx_data_len;

static CanDatagramTxConfig s_tx_config = {
  .dgram_type = BOOTLOADER_DATAGRAM_FLASH_APPLICATION_META,
  .destination_nodes_len = 1,
  .destination_nodes = &s_client_id,
  .data = s_tx_data,
  .tx_cmpl_cb = tx_cmpl_cb,
};

static CanDatagramRxConfig s_rx_config = {
  .destination_nodes = s_destination_nodes,
  .data = s_rx_data,
  .node_id = 0,  // listen to all
  .rx_cmpl_cb = NULL,
};

static BootloaderConfig s_bootloader_config = { 0 };

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  gpio_init();
  soft_timer_init();
  crc32_init();
  flash_init();
  config_init();

  ms_test_helper_datagram_init(&s_test_can_storage, &s_test_can_settings, s_board_id,
                               &s_test_datagram_settings);
  dispatcher_init(s_board_id);
}

void teardown_test(void) {}

static size_t prv_strnlen(const char *str, size_t maxlen) {
  size_t i;
  for (i = 0; i < maxlen; ++i) {
    if (str[i] == '\0') {
      break;
    }
  }
  return i;
}

static bool prv_encode_string(pb_ostream_t *stream, const pb_field_iter_t *field,
                              void *const *arg) {
  const char *str = *arg;
  // add to stream
  if (!pb_encode_tag_for_field(stream, field)) {  // write tag and wire type
    return false;
  }
  return pb_encode_string(stream, (uint8_t *)str,
                          prv_strnlen(str, MAX_STRING_SIZE));  // write sting
}

void test_flash_application_code_invalid_crc(void) {
  flash_application_init();

  char name[] = "new name";
  char git_version[] = "a";

  FlashApplicationCode metadata;
  metadata.name.funcs.encode = prv_encode_string;
  metadata.git_version.funcs.encode = prv_encode_string;
  metadata.name.arg = name;
  metadata.git_version.arg = git_version;
  metadata.application_crc = 0x150088C2;
  metadata.size = 5 * SIZEOF_ARRAY(s_tx_data);

  pb_ostream_t pb_ostream = pb_ostream_from_buffer(s_tx_data, (size_t)PROTOBUF_MAXSIZE);
  pb_encode(&pb_ostream, FlashApplicationCode_fields, &metadata);

  s_tx_config.dgram_type = BOOTLOADER_DATAGRAM_FLASH_APPLICATION_META;
  s_tx_config.data_len = pb_ostream.bytes_written;

  dgram_helper_mock_tx_datagram(&s_tx_config);
  dgram_helper_mock_rx_datagram(&s_rx_config);

  TEST_ASSERT_EQUAL(1, s_rx_config.data_len);
  TEST_ASSERT_OK(s_rx_config.data[0]);

  LOG_DEBUG("completed application metadata transfer\n");

  s_tx_config.dgram_type = BOOTLOADER_DATAGRAM_FLASH_APPLICATION_DATA;
  s_tx_config.data_len = SIZEOF_ARRAY(s_tx_data);

  uint8_t byte_val = 0;
  for (uint16_t page = 0; page < 4; ++page) {
    // set up the 2048 length data
    for (uint16_t i = 0; i < SIZEOF_ARRAY(s_tx_data); ++i) {
      s_tx_data[i] = byte_val;
      byte_val = (byte_val + 1) % 231;
    }

    dgram_helper_mock_tx_datagram(&s_tx_config);
    dgram_helper_mock_rx_datagram(&s_rx_config);

    TEST_ASSERT_EQUAL(1, s_rx_config.data_len);
    TEST_ASSERT_EQUAL_MESSAGE(STATUS_CODE_OK, s_rx_config.data[0], "sending page failed");
  }

  // send the last page
  for (uint16_t i = 0; i < SIZEOF_ARRAY(s_tx_data); ++i) {
    s_tx_data[i] = byte_val;
    byte_val = (byte_val + 1) % 231;
  }

  dgram_helper_mock_tx_datagram(&s_tx_config);
  dgram_helper_mock_rx_datagram(&s_rx_config);

  TEST_ASSERT_EQUAL(1, s_rx_config.data_len);
  TEST_ASSERT_EQUAL_MESSAGE(STATUS_CODE_INTERNAL_ERROR, s_rx_config.data[0],
                            "crc check did not occur");

  uint16_t first_page = FLASH_ADDR_TO_PAGE(BOOTLOADER_APPLICATION_START);
  byte_val = 0;
  for (uint16_t page = 0; page < 5; ++page) {
    flash_read(FLASH_PAGE_TO_ADDR(first_page + page), FLASH_PAGE_BYTES, s_tx_data,
               SIZEOF_ARRAY(s_tx_data));

    for (uint16_t i = 0; i < FLASH_PAGE_BYTES; ++i) {
      TEST_ASSERT_EQUAL(byte_val, s_tx_data[i]);
      byte_val = (byte_val + 1) % 231;
    }
  }
  char empty[64] = { 0 };
  TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(empty, s_bootloader_config.project_name, 64,
                                       "project name was changed when crc is wrong");
  TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(empty, s_bootloader_config.git_version, 64,
                                       "git version was changed when crc is wrong");
}

void test_flash_application_code(void) {
  flash_application_init();

  char name[] = "new name";
  char git_version[] = "a";

  FlashApplicationCode metadata;
  metadata.name.funcs.encode = prv_encode_string;
  metadata.git_version.funcs.encode = prv_encode_string;
  metadata.name.arg = name;
  metadata.git_version.arg = git_version;
  metadata.application_crc = 0x150088C1;
  metadata.size = 5 * SIZEOF_ARRAY(s_tx_data);

  pb_ostream_t pb_ostream = pb_ostream_from_buffer(s_tx_data, (size_t)PROTOBUF_MAXSIZE);
  pb_encode(&pb_ostream, FlashApplicationCode_fields, &metadata);

  s_tx_config.dgram_type = BOOTLOADER_DATAGRAM_FLASH_APPLICATION_META;
  s_tx_config.data_len = pb_ostream.bytes_written;

  dgram_helper_mock_tx_datagram(&s_tx_config);
  dgram_helper_mock_rx_datagram(&s_rx_config);

  TEST_ASSERT_EQUAL(1, s_rx_config.data_len);
  TEST_ASSERT_OK(s_rx_config.data[0]);

  LOG_DEBUG("completed application metadata transfer\n");

  s_tx_config.dgram_type = BOOTLOADER_DATAGRAM_FLASH_APPLICATION_DATA;
  s_tx_config.data_len = SIZEOF_ARRAY(s_tx_data);

  uint8_t byte_val = 0;
  for (uint16_t page = 0; page < 5; ++page) {
    // set up the 2048 length data
    for (uint16_t i = 0; i < SIZEOF_ARRAY(s_tx_data); ++i) {
      s_tx_data[i] = byte_val;
      byte_val = (byte_val + 1) % 231;
    }

    dgram_helper_mock_tx_datagram(&s_tx_config);
    dgram_helper_mock_rx_datagram(&s_rx_config);

    TEST_ASSERT_EQUAL(1, s_rx_config.data_len);
    TEST_ASSERT_EQUAL_MESSAGE(STATUS_CODE_OK, s_rx_config.data[0], "sending page failed");
  }

  uint16_t first_page = FLASH_ADDR_TO_PAGE(BOOTLOADER_APPLICATION_START);
  byte_val = 0;
  for (uint16_t page = 0; page < 5; ++page) {
    flash_read(FLASH_PAGE_TO_ADDR(first_page + page), FLASH_PAGE_BYTES, s_tx_data,
               SIZEOF_ARRAY(s_tx_data));

    for (uint16_t i = 0; i < FLASH_PAGE_BYTES; ++i) {
      TEST_ASSERT_EQUAL(byte_val, s_tx_data[i]);
      byte_val = (byte_val + 1) % 231;
    }
  }

  config_get(&s_bootloader_config);

  TEST_ASSERT_EQUAL_STRING_LEN(name, s_bootloader_config.project_name, SIZEOF_ARRAY(name));
  TEST_ASSERT_EQUAL_STRING_LEN(git_version, s_bootloader_config.git_version,
                               SIZEOF_ARRAY(git_version));
}
