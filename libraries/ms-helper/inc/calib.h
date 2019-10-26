#pragma once
// Calibration persistence - uses the flash persist layer
// Any changes must be explicitly committed.
// There can only be one instance of this module.
#include "persist.h"

// Use the last flash page for calibration persistence
#define CALIB_FLASH_PAGE NUM_FLASH_PAGES - 1

// Load any stored data into |blob|. |blob| should persist.
StatusCode calib_init(void *blob, size_t blob_size, bool overwrite);

// Store any changes to the calibration blob.
StatusCode calib_commit(void);

// Retrieve persisted calibration data.
void *calib_blob(void);
