#pragma once
// Implements a flash-based persistance layer
// Requires flash, soft timers, and CRC32 to be initialized.
//
// Allocates one page of flash to keep data across resets.
// Data is written to flash periodically. Note that to reduce necessary wear on
// the flash, data is only committed if changes have occurred.
//
// Due to the way this is implemented, multiple persistance instances can be
// active at once. However, note that there is no protection against multiple
// persistance instances writing to the same page.
#include <stddef.h>
#include "flash.h"
#include "soft_timer.h"
#include "status.h"

// Commit data every second if dirty
#define PERSIST_COMMIT_TIMEOUT_MS 1000

typedef struct PersistStorage {
  void *blob;
  size_t blob_size;
  uintptr_t flash_addr;
  uintptr_t prev_flash_addr;
  uint32_t prev_hash;
  FlashPage page;
  SoftTimerId timer_id;
} PersistStorage;

// Attempt to load stored data into the provided blob and retains the blob to
// commit periodically Reserves the entire flash page for the persistance layer
// instance Note that the blob must be a multiple of FLASH_WRITE_BYTES and must
// persist If |overwrite| is true, the persist layer overwrites invalid blobs.
// Otherwise, it fails.
StatusCode persist_init(PersistStorage *persist, FlashPage page, void *blob, size_t blob_size,
                        bool overwrite);

// Control whether the periodic commit is enabled (enabled by default)
StatusCode persist_ctrl_periodic(PersistStorage *persist, bool enabled);

// Force a data commit - this should be avoided if possible.
StatusCode persist_commit(PersistStorage *persist);
