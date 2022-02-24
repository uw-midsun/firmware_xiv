#include "config.h"
#include "bootloader_mcu.h"
#include "crc32.h"
#include "flash.h"
#include "log.h"
#include "persist.h"
#include "soft_timer.h"

// 63 is the maximum controller board ID, so it is easy to tell if it is invalid by setting it to 63
#define DEFAULT_CONTROLLER_BOARD_ID 63

#define BUFFER_LEN 2048

static PersistStorage s_config_1_persist = { 0 };
static BootloaderConfig s_config_1_blob = { .crc32 = 0,
                                            .controller_board_id = DEFAULT_CONTROLLER_BOARD_ID,
                                            .controller_board_name = "<unset>",
                                            .project_present = false,
                                            .project_name = "",
                                            .project_info = "",
                                            .git_version = "",
                                            .application_crc32 = 0,
                                            .application_size = 0 };

static PersistStorage s_config_2_persist = { 0 };
static BootloaderConfig s_config_2_blob = { .crc32 = 0,
                                            .controller_board_id = DEFAULT_CONTROLLER_BOARD_ID,
                                            .controller_board_name = "<unset>",
                                            .project_present = false,
                                            .project_name = "",
                                            .project_info = "",
                                            .git_version = "",
                                            .application_crc32 = 0,
                                            .application_size = 0 };

// This sets a type flash page to the config pages from bootloader_mcu
#define BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE (FLASH_ADDR_TO_PAGE(BOOTLOADER_CONFIG_PAGE_1_START))
#define BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE (FLASH_ADDR_TO_PAGE(BOOTLOADER_CONFIG_PAGE_2_START))

static uint32_t prv_compute_crc32(BootloaderConfig *config) {
  uint32_t temp_crc = config->crc32;
  config->crc32 = 0;
  uint32_t calculated_crc = crc32_arr((uint8_t *)config, sizeof(BootloaderConfig));
  config->crc32 = temp_crc;
  return calculated_crc;
}

static uint32_t prv_compute_application_crc32() {
  // code array for holding crc32 codes per 2048 bytes
  // size of array is BOOTLOADER_APPLICATION_SIZE / BUFFER_LEN + 1
  uint8_t crc32_codes[(BOOTLOADER_APPLICATION_SIZE / BUFFER_LEN + 1) * 4];
  uint8_t buffer_bytes_size = 0;
  uint8_t crc32_code_number = 0;
  size_t curr_size = 0;        // memory size that has been read
  uint8_t buffer[BUFFER_LEN];  // buffer for holding 2048 bytes of flash
  uint32_t crc_temp = 0;
  uint8_t firstByte = 0, secondByte = 0, thirdByte = 0, fourthByte = 0;

  while (curr_size < BOOTLOADER_APPLICATION_SIZE) {
    // read from flash
    flash_read((uintptr_t)BOOTLOADER_APPLICATION_START + curr_size, sizeof(buffer[0]) * BUFFER_LEN,
               buffer, sizeof(buffer[0]) * BUFFER_LEN);

    // calculate crc32
    crc_temp = crc32_arr(buffer, sizeof(buffer[0]) * BUFFER_LEN);

    // extact bytes of each buffer value in little endian form
    firstByte = (crc_temp << 24) >> 24;
    crc32_codes[crc32_code_number] = firstByte;

    secondByte = (crc_temp << 16) >> 24;
    crc32_codes[crc32_code_number + 1] = secondByte;

    thirdByte = (crc_temp << 8) >> 24;
    crc32_codes[crc32_code_number + 2] = thirdByte;

    fourthByte = (crc_temp) >> 24;
    crc32_codes[crc32_code_number + 3] = fourthByte;

    crc32_code_number += 4;
    curr_size += sizeof(buffer);
  }

  // calculate the final crc32 and return it
  return crc32_arr(crc32_codes, sizeof(crc32_codes[0]) * crc32_code_number);
}

StatusCode config_init(void) {
  // This function uses persist_init to pull the flash page and puts it into the type
  // BootloaderConfig blob

  // It makes sure that both config pages will not end up corrupted
  // The safety functions are that if a page is corrupted but the other is not then
  // the "safe" page is copied over the corrupted page
  // If both are corrupted then a critical error is returned

  s_config_1_blob.crc32 = prv_compute_crc32(&s_config_1_blob);
  s_config_2_blob.crc32 = prv_compute_crc32(&s_config_2_blob);

  // persist_init pulls the flash page and puts it into the type BootloaderConfig blob
  // persist_init pulls the flash page and puts it into the type BootloaderConfig blob
  status_ok_or_return(persist_init(&s_config_1_persist, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE,
                                   &s_config_1_blob, sizeof(BootloaderConfig), false));
  status_ok_or_return(persist_ctrl_periodic(&s_config_1_persist, false));

  status_ok_or_return(persist_init(&s_config_2_persist, BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE,
                                   &s_config_2_blob, sizeof(BootloaderConfig), false));
  status_ok_or_return(persist_ctrl_periodic(&s_config_2_persist, false));

  uint32_t config_1_check_crc = 0;
  uint32_t config_2_check_crc = 0;

  // Used for comparison only
  BootloaderConfig s_config_empty = { 0 };

  // Check for if the bootloader config is still zero. If it is not, then calculate the crc of the
  // config to fill in the static configs
  if (memcmp(&s_config_1_blob, &s_config_empty, sizeof(BootloaderConfig)) != 0) {
    config_1_check_crc = prv_compute_crc32(&s_config_1_blob);
  }

  if (memcmp(&s_config_2_blob, &s_config_empty, sizeof(BootloaderConfig)) != 0) {
    config_2_check_crc = prv_compute_crc32(&s_config_2_blob);
  }

  // checks to see if there are differences in crc to detect corruption
  bool is_config_1_corrupted = !(s_config_1_blob.crc32 == config_1_check_crc);
  bool is_config_2_corrupted = !(s_config_2_blob.crc32 == config_2_check_crc);

  // memcpy copies one page to the other ("restoring" the corrupted page)
  if (is_config_1_corrupted && is_config_2_corrupted) {
    LOG_CRITICAL("ERROR: Both config pages corrupted \n");
    return STATUS_CODE_INTERNAL_ERROR;
  } else if (is_config_2_corrupted) {
    memcpy(&s_config_2_blob, &s_config_1_blob, sizeof(BootloaderConfig));
    persist_commit(&s_config_2_persist);
  } else if (is_config_1_corrupted || config_1_check_crc != config_2_check_crc) {
    memcpy(&s_config_1_blob, &s_config_2_blob, sizeof(BootloaderConfig));
    persist_commit(&s_config_1_persist);
  }
  return STATUS_CODE_OK;
}

void config_get(BootloaderConfig *config) {
  // Please persist any changes
  if (config == NULL) {
    return;
  }
  memcpy(config, &s_config_1_blob, sizeof(BootloaderConfig));
}

StatusCode config_commit(BootloaderConfig *input_config) {
  // This function sets the current config page 1 to the input config
  // To prevent corruption, if page 1 is corrupted during the transfer, then
  // page 2 is immediately copied over to page 1 and an error is returned
  // If page 1 is not corrupted then page 1 is copied over to page 2

  if (input_config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  input_config->crc32 = prv_compute_crc32(input_config);
  input_config->application_crc32 = prv_compute_application_crc32(input_config);

  // This memcpy copies over the input config onto the config page 1
  // This copy is commited into the storage, to change s_config_1_blob
  // a persist_init will be called
  memcpy(&s_config_1_blob, input_config, sizeof(BootloaderConfig));
  persist_commit(&s_config_1_persist);

  // This persist_init updates the s_config_1_blob by pulling from the flash
  status_ok_or_return(persist_init(&s_config_1_persist, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE,
                                   &s_config_1_blob, sizeof(BootloaderConfig), false));
  status_ok_or_return(persist_ctrl_periodic(&s_config_1_persist, false));

  // checks to see if blob 1 and input config are different to detect corruption
  bool config_1_corrupted = (memcmp(&s_config_1_blob, input_config, sizeof(BootloaderConfig)) != 0);

  if (!config_1_corrupted) {
    memcpy(&s_config_2_blob, &s_config_1_blob, sizeof(BootloaderConfig));
    persist_commit(&s_config_2_persist);
  } else {
    memcpy(&s_config_1_blob, &s_config_2_blob, sizeof(BootloaderConfig));
    persist_commit(&s_config_1_persist);
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return STATUS_CODE_OK;
}
