#include "flash_application_code.h"

#include "stdint.h"

// process:
// 1. listen for datagram id 8
// 2. respond
// 3. listen for datagram id 9
// 4. flash
// 5. respond (waits for flash to finish)
// 5. repeat 3 - 5

static uint16_t remaining_size;

static void prv_start_flash();

static void prv_flash();

static void prv_reset();
