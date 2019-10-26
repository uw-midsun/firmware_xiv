#include "generic_can_uart.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "can_uart.h"
#include "generic_can.h"
#include "generic_can_helpers.h"
#include "status.h"

static CanUart s_can_uart;
static GenericCanInterface s_interface;

// CanUartRxCb
static void prv_generic_can_uart_rx_handler(const CanUart *can_uart, uint32_t id, bool extended,
                                            const uint64_t *data, size_t dlc, void *context) {
  (void)can_uart;
  GenericCanUart *gcu = context;
  for (size_t i = 0; i < NUM_GENERIC_CAN_RX_HANDLERS; i++) {
    if (gcu->base.rx_storage[i].rx_handler != NULL &&
        (id & gcu->base.rx_storage[i].mask) == gcu->base.rx_storage[i].filter) {
      const GenericCanMsg msg = {
        .id = id,
        .extended = extended,
        .data = *data,
        .dlc = dlc,
      };
      gcu->base.rx_storage[i].rx_handler(&msg, gcu->base.rx_storage[i].context);
      break;
    }
  }
}

// tx
static StatusCode prv_tx(const GenericCan *can, const GenericCanMsg *msg) {
  GenericCanUart *gcu = (GenericCanUart *)can;
  if (gcu->base.interface != &s_interface || gcu->can_uart != &s_can_uart) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanUart.");
  }
  return can_uart_req_slave_tx(gcu->can_uart, msg->id, msg->extended, &msg->data, msg->dlc);
}

// register_rx
static StatusCode prv_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t mask,
                                  uint32_t filter, bool extended, void *context) {
  (void)extended;
  GenericCanUart *gcu = (GenericCanUart *)can;
  if (gcu->base.interface != &s_interface || gcu->can_uart != &s_can_uart) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanUart.");
  }
  return generic_can_helpers_register_rx(can, rx_handler, mask, filter, context, NULL);
}

StatusCode generic_can_uart_init(GenericCanUart *can_uart, UartPort port) {
  s_interface.tx = prv_tx;
  s_interface.register_rx = prv_register_rx;

  s_can_uart.uart = port;
  s_can_uart.context = (void *)can_uart;
  s_can_uart.rx_cb = prv_generic_can_uart_rx_handler;
  status_ok_or_return(can_uart_init(&s_can_uart));
  can_uart->can_uart = &s_can_uart;

  memset(can_uart->base.rx_storage, 0, sizeof(GenericCanRx) * NUM_GENERIC_CAN_RX_HANDLERS);

  can_uart->base.interface = &s_interface;
  return STATUS_CODE_OK;
}
