#pragma once

// This module handles the bootloader config
// Requires flash, soft timers, and CRC32 to be initialized.

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "bootloader_mcu.h"
#include "persist.h"
#include "status.h"

typedef struct BootloaderConfig {
  uint32_t CRC32;
  uint8_t controller_board_ID;
  char controller_board_name[64];
  bool project_present;
  char project_name[64];
  char project_info[64];
  char git_version[64];
  uint32_t app_code;
  uint32_t app_size;
} BootloaderConfig;

// Initializes the config and will return a STATUS_CODE_INTERNAL_ERROR if both pages are corrupted
// Additionally, it will check if the two redundant pages are the same and will correct if one is
// corrupted
StatusCode config_init(void);

// Gets the config for page 1, and memcpys it to the input config pointer
// Please persist any changes you make
StatusCode config_get(BootloaderConfig *config);

// Will add the input config as the new config
// If the config is corrupted during transfer, then the
// module will use the old redundant page to "reset" the config
StatusCode config_commit(BootloaderConfig *input_config);
