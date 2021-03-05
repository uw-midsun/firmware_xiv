#include "config.h"
#include "crc32.h"
#include "flash.h"
#include "log.h"
#include "persist.h"
#include "soft_timer.h"

static PersistStorage s_config_1_persist = { 0 };
static BootloaderConfig s_config_1_blob = { 0 };

static PersistStorage s_config_2_persist = { 0 };
static BootloaderConfig s_config_2_blob = { 0 };

#define BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE (FLASH_ADDR_TO_PAGE(BOOTLOADER_CONFIG_PAGE_1_START))
#define BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE (FLASH_ADDR_TO_PAGE(BOOTLOADER_CONFIG_PAGE_2_START))

StatusCode config_init(void) {
  status_ok_or_return(persist_init(&s_config_1_persist, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE,
                                   &s_config_1_blob, sizeof(BootloaderConfig), false));
  status_ok_or_return(persist_ctrl_periodic(&s_config_1_persist, false));

  status_ok_or_return(persist_init(&s_config_2_persist, BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE,
                                   &s_config_2_blob, sizeof(BootloaderConfig), false));
  status_ok_or_return(persist_ctrl_periodic(&s_config_2_persist, false));

  uint32_t config_1_crc = s_config_1_blob.crc32;
  s_config_1_blob.crc32 = 0;

  uint32_t config_2_crc = s_config_2_blob.crc32;
  s_config_2_blob.crc32 = 0;

  uint32_t config_1_check_crc = crc32_arr((uint8_t *)&s_config_1_blob, sizeof(BootloaderConfig));
  uint32_t config_2_check_crc = crc32_arr((uint8_t *)&s_config_2_blob, sizeof(BootloaderConfig));

  bool is_config_1_equal = config_1_crc == config_1_check_crc;
  bool is_config_2_equal = config_2_crc == config_2_check_crc;

  if (is_config_1_equal && is_config_2_equal) {
    s_config_1_blob.crc32 = config_1_crc;
    s_config_2_blob.crc32 = config_2_crc;
  } else if (!is_config_1_equal && !is_config_2_equal) {
    LOG_CRITICAL("ERROR: Both config pages corrupted \n");
    return STATUS_CODE_INTERNAL_ERROR;
  } else if (is_config_1_equal) {
    s_config_1_blob.crc32 = config_1_crc;
    memcpy(&s_config_2_blob, &s_config_1_blob, sizeof(BootloaderConfig));
    persist_commit(&s_config_2_persist);
  } else {
    s_config_2_blob.crc32 = config_2_crc;
    memcpy(&s_config_1_blob, &s_config_2_blob, sizeof(BootloaderConfig));
    persist_commit(&s_config_1_persist);
  }

  return STATUS_CODE_OK;
}

void config_get(BootloaderConfig *config) {
  // Please persist any changes
  memcpy(config, &s_config_1_blob, sizeof(BootloaderConfig));
}

StatusCode config_commit(BootloaderConfig *input_config) {
  input_config->crc32 = 0;
  uint32_t input_config_crc = crc32_arr((uint8_t *)input_config, sizeof(BootloaderConfig));
  input_config->crc32 = input_config_crc;

  memcpy(&s_config_1_blob, input_config, sizeof(BootloaderConfig));
  persist_commit(&s_config_1_persist);

  status_ok_or_return(persist_init(&s_config_1_persist, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE,
                                   &s_config_1_blob, sizeof(BootloaderConfig), false));

  bool input_equals_config_1 = memcmp(&s_config_1_blob, input_config, sizeof(BootloaderConfig)) == 0;

  if (input_equals_config_1) {
    memcpy(&s_config_2_blob, &s_config_1_blob, sizeof(BootloaderConfig));
    persist_commit(&s_config_2_persist);
  } else {
    memcpy(&s_config_1_blob, &s_config_2_blob, sizeof(BootloaderConfig));
    persist_commit(&s_config_1_persist);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}
