#include "generic_can.h"

#include <stdbool.h>
#include <stdint.h>

#include "generic_can_msg.h"
#include "status.h"

StatusCode generic_can_tx(const GenericCan *can, const GenericCanMsg *msg) {
  return can->interface->tx(can, msg);
}

StatusCode generic_can_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t mask,
                                   uint32_t filter, bool extended, void *context) {
  return can->interface->register_rx(can, rx_handler, mask, filter, extended, context);
}
