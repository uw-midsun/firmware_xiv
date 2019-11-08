#include "generic_can_mcp2515.h"
#include <string.h>
#include "generic_can_helpers.h"

static Mcp2515Storage s_mcp2515;
static GenericCanInterface s_interface;

static void prv_rx_handler(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context) {
  GenericCanMcp2515 *gcmcp = context;
  for (size_t i = 0; i < NUM_GENERIC_CAN_RX_HANDLERS; i++) {
    if (gcmcp->base.rx_storage[i].rx_handler != NULL &&
        (id & gcmcp->base.rx_storage[i].mask) == gcmcp->base.rx_storage[i].filter) {
      const GenericCanMsg msg = {
        .id = id,
        .extended = extended,
        .data = data,
        .dlc = dlc,
      };
      gcmcp->base.rx_storage[i].rx_handler(&msg, gcmcp->base.rx_storage[i].context);
      break;
    }
  }
}

static StatusCode prv_tx(const GenericCan *can, const GenericCanMsg *msg) {
  GenericCanMcp2515 *gcmcp = (GenericCanMcp2515 *)can;
  if (gcmcp->base.interface != &s_interface || gcmcp->mcp2515 != &s_mcp2515) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanMcp2515.");
  }
  return mcp2515_tx(gcmcp->mcp2515, msg->id, msg->extended, msg->data, msg->dlc);
}

static StatusCode prv_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t mask,
                                  uint32_t filter, bool extended, void *context) {
  GenericCanMcp2515 *gcmcp = (GenericCanMcp2515 *)can;
  if (gcmcp->base.interface != &s_interface || gcmcp->mcp2515 != &s_mcp2515) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanMcp2515.");
  }
  return generic_can_helpers_register_rx(can, rx_handler, mask, filter, context, NULL);
}

StatusCode generic_can_mcp2515_init(GenericCanMcp2515 *can_mcp2515,
                                    const Mcp2515Settings *settings) {
  s_interface.tx = prv_tx;
  s_interface.register_rx = prv_register_rx;

  status_ok_or_return(mcp2515_init(&s_mcp2515, settings));
  status_ok_or_return(mcp2515_register_rx_cb(&s_mcp2515, prv_rx_handler, can_mcp2515));
  can_mcp2515->mcp2515 = &s_mcp2515;

  memset(can_mcp2515->base.rx_storage, 0, sizeof(GenericCanRx) * NUM_GENERIC_CAN_RX_HANDLERS);

  can_mcp2515->base.interface = &s_interface;
  return STATUS_CODE_OK;
}
