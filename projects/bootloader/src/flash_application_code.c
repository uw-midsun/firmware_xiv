#include "flash_application_code.h"

#include <stdint.h>

#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "bootloader_mcu.h"
#include "can_datagram.h"
#include "config.h"
#include "crc32.h"
#include "dispatcher.h"
#include "flash.h"
#include "flash_application_code.pb.h"
#include "log.h"
#include "misc.h"
#include "pb_decode.h"
#include "reset.h"
#include "status.h"

static FlashApplicationCode s_meta_data = FlashApplicationCode_init_default;
static FlashPage s_page;
static uint32_t s_app_crc;
static uint32_t s_remaining_size;
static StatusCode status;

static char name[64];
static char git_version[64];

static bool prv_decode_string(pb_istream_t *stream, const pb_field_iter_t *field, void **arg) {
  size_t str_len = MIN(stream->bytes_left, (size_t)64);
  strncpy((char *)*arg, (char *)stream->state, str_len);
  return true;
}

static StatusCode prv_start_flash(uint8_t *data, uint16_t data_len, void *context) {
  pb_istream_t pb_istream = pb_istream_from_buffer(data, data_len);
  if (!pb_decode(&pb_istream, FlashApplicationCode_fields, &s_meta_data)) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_app_crc = 0;
  s_remaining_size = s_meta_data.size;
  s_page = FLASH_ADDR_TO_PAGE(BOOTLOADER_APPLICATION_START);

  LOG_DEBUG("recieved flash metadata");

  return STATUS_CODE_OK;
}

static StatusCode prv_flash_complete() {
  BootloaderConfig s_updated_config;
  config_get(&s_updated_config);
  if (s_app_crc != s_meta_data.application_crc) {
    // crc does not match
    strncpy(s_updated_config.project_name, "no project", 64);
    strncpy(s_updated_config.git_version, "", 64);

    config_commit(&s_updated_config);
    return status_response(STATUS_CODE_INTERNAL_ERROR, tx_cmpl_cb);
  } else {
    strncpy(s_updated_config.project_name, name, 64);
    strncpy(s_updated_config.git_version, git_version, 64);
    s_updated_config.application_crc32 = s_meta_data.application_crc;
    s_updated_config.application_size = s_meta_data.size;

    return status_response(config_commit(&s_updated_config), reset);
  }
}

static StatusCode prv_flash_page(uint8_t *data, uint16_t data_len, void *context) {
  // # This process assumes |data_len| less than FLASH_PAGE_BYTES #
  if (s_remaining_size < data_len) {
    return STATUS_CODE_OUT_OF_RANGE;
  }
  // flash page, set data_len to the next multiple of 4
  // |flash_write_len| will never exceed DATA_LEN_MAX
  uint16_t flash_write_len = (data_len + 3) / FLASH_MCU_WRITE_BYTES * FLASH_MCU_WRITE_BYTES;
  LOG_DEBUG("%i, %i\n", flash_write_len, data_len);

  // erase then write data on page
  for (uint16_t i = 0; i < data_len; i += FLASH_PAGE_BYTES) {
    status_ok_or_return(flash_erase(s_page + i / FLASH_PAGE_BYTES));
  }

  status_ok_or_return(flash_write(FLASH_PAGE_TO_ADDR(s_page), data, flash_write_len));

  for (uint16_t i = 0; i < data_len; i += FLASH_PAGE_BYTES) {
    s_page++;
  }

  // add to crc
  s_app_crc = crc32_append_arr(data, data_len, s_app_crc);

  s_remaining_size -= data_len;
  if (s_remaining_size <= 0) {
    return prv_flash_complete();
  }

  return STATUS_CODE_OK;
}

static StatusCode prv_flash_page_with_response(uint8_t *data, uint16_t data_len, void *context) {
  status_response(prv_flash_page(data, data_len, context));
}

StatusCode flash_application_init() {
  s_meta_data.git_version.arg = git_version;
  s_meta_data.name.arg = name;
  s_meta_data.git_version.funcs.decode = prv_decode_string;
  s_meta_data.name.funcs.decode = prv_decode_string;

  status_ok_or_return(dispatcher_register_callback(BOOTLOADER_DATAGRAM_FLASH_APPLICATION_META,
                                                   prv_start_flash, NULL));
  status_ok_or_return(dispatcher_register_callback(BOOTLOADER_DATAGRAM_FLASH_APPLICATION_DATA,
                                                   prv_flash_page_with_response, NULL));
  return STATUS_CODE_OK;
}
