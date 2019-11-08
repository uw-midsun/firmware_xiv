#pragma once
// Helpers for common code used in generic_can implementations. This interface
// is intended to be
// **private** to generic_can*.c files and it should never be included in a *.h
// file.

#include <stdbool.h>
#include <stdint.h>

#include "generic_can.h"
#include "status.h"

// NOTE: Callers are expected to validate and sanitize |can| to avoid
// dereferencing null or other out of bounds memory!

// Registers |rx_handler| for |id| to |can| which will be passed |context| when
// triggered.
StatusCode generic_can_helpers_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t mask,
                                           uint32_t filter, void *context, uint16_t *idx);
