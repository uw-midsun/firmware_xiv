#include "flash_application_code.h"

#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "can_datagram.h"
#include "crc32.h"
#include "dispatcher.h"
#include "flash.h"
#include "flash_application_code.pb.h"
#include "pb_decode.h"
#include "stdint.h"

// process:
// 1. listen for datagram id 8
// 2. respond
// 3. listen for datagram id 9
// 4. flash
// 5. respond (waits for flash to finish)
// 5. repeat 3 - 5

#define APPLICATION_START_PAGE 2  // start of the application page in flash

static FlashApplicationCode s_meta_data = FlashApplicationCode_init_default;
static FlashPage s_page = APPLICATION_START_PAGE;
static uint8_t total_crc = 0;

static StatusCode prv_start_flash(uint8_t *data, uint16_t data_len, void *context) {
  pb_istream_t pb_istream = pb_istream_from_buffer(data, data_len);
  if (!pb_decode(&pb_istream, FlashApplicationCode_fields, &s_meta_data)) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

static StatusCode prv_finish_flash() {}

static StatusCode prv_flash_page(uint8_t *data, uint16_t data_len, void *context) {
  // # This process assumes |data_len| less than FLASH_PAGE_BYTES #

  // erase page
  flash_erase(s_page);

  // flash page, set data_len to the next multiple of 4
  // |flash_write_len| will never exceed DATA_LEN_MAX
  uint16_t flash_write_len = (data_len + 3) / FLASH_MCU_WRITE_BYTES * FLASH_MCU_WRITE_BYTES;

  flash_write(FLASH_PAGE_TO_ADDR(s_page), data, flash_write_len);

  s_page++;
  // add to crc
  uint32_t page_crc = crc32_arr(data, data_len);
  // return STATUS_CODE_EMPTY;
}

static void prv_decode_string(pb_istream_t *stream, const pb_field_iter_t *field, void **arg) {
  strncpy(*arg, stream->state, 64);
}

static void prv_reset();

StatusCode flash_application_init() {
  s_meta_data.git_version.funcs.decode = prv_decode_string;
  s_meta_data.name.funcs.decode = prv_decode_string;

  {
    StatusCode i = STATUS_CODE_EMPTY;
    if (i) return i;
  }
  status_ok_or_return(STATUS_CODE_EMPTY);
  status_ok_or_return(dispatcher_register_callback(BOOTLOADER_DATAGRAM_FLASH_APPLICATION_DATA,
                                                   prv_flash_page, NULL));
}
