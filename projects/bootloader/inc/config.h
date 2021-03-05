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
  uint32_t crc32;                  // CRC32 of config blob
  uint8_t controller_board_id;     // numeric ID of the controller board
  char controller_board_name[64];  // human-friendly name of the controller board
  bool project_present;            // is there an application (project) present?
  char project_name[64];           // name of the current project (empty project is "no project")
  char project_info[64];           // possible extra info to differentiate boards
  char git_version[64];            // commit hash of the branch we flashed from
  uint32_t application_crc32;      // CRC32 of the application code
  uint32_t application_size;       // size of the application code that the bootloader boots into
} BootloaderConfig;

// Initializes the config and will return a STATUS_CODE_INTERNAL_ERROR if both pages are corrupted
// Additionally, it will check if the two redundant pages are the same and will correct if one is
// corrupted
StatusCode config_init(void);

// Gets the config for page 1, and memcpys it to the input config pointer
// Please persist any changes you make
void config_get(BootloaderConfig *config);

// Will add the input config as the new config
// If the config is corrupted during transfer, then the
// module will use the old redundant page to "reset" the config
StatusCode config_commit(BootloaderConfig *input_config);
