#include "generic_can_helpers.h"

#include <stdbool.h>
#include <stdint.h>

#include "generic_can.h"
#include "status.h"

StatusCode generic_can_helpers_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t mask,
                                           uint32_t filter, void *context, uint16_t *idx) {
  for (size_t i = 0; i < NUM_GENERIC_CAN_RX_HANDLERS; i++) {
    if (can->rx_storage[i].rx_handler == NULL) {
      can->rx_storage[i].mask = mask;
      can->rx_storage[i].filter = filter;
      can->rx_storage[i].rx_handler = rx_handler;
      can->rx_storage[i].context = context;
      if (idx != NULL) {
        *idx = i;
      }
      return STATUS_CODE_OK;
    }
  }
  return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
}
