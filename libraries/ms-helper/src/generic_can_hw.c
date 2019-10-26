#include "generic_can_hw.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "can_hw.h"
#include "event_queue.h"
#include "generic_can.h"
#include "generic_can_helpers.h"
#include "generic_can_msg.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"

#define CAN_BUS_OFF_RECOVERY_TIME_MS 500

static GenericCanInterface s_interface;

// CanHwEventHandlerCb: Tx Occurred (dummy to avoid segfault if not present).
static void prv_tx_handler(void *context) {
  (void)context;
}

// CanHwEventHandlerCb: Rx Occurred
static void prv_rx_handler(void *context) {
  GenericCanHw *gch = (GenericCanHw *)context;
  GenericCanMsg rx_msg = { 0 };
  while (can_hw_receive(&rx_msg.id, &rx_msg.extended, &rx_msg.data, &rx_msg.dlc)) {
    for (size_t i = 0; i < NUM_GENERIC_CAN_RX_HANDLERS; i++) {
      if (gch->base.rx_storage[i].rx_handler != NULL &&
          (rx_msg.id & gch->base.rx_storage[i].mask) == gch->base.rx_storage[i].filter) {
        gch->base.rx_storage[i].rx_handler(&rx_msg, gch->base.rx_storage[i].context);
        break;
      }
    }
  }
}

// CanHwEventHandlerCb: Fault Occurred
static void prv_bus_error_timeout_handler(SoftTimerId timer_id, void *context) {
  (void)timer_id;
  GenericCanHw *gch = context;

  // Note that bus errors have never been tested.
  CanHwBusStatus status = can_hw_bus_status();

  if (status == CAN_HW_BUS_STATUS_OFF) {
    event_raise(gch->fault_event, 0);
  }
}

static void prv_bus_error_handler(void *context) {
  GenericCanHw *gch = context;

  soft_timer_start_millis(CAN_BUS_OFF_RECOVERY_TIME_MS, prv_bus_error_timeout_handler, gch, NULL);
}

// tx
static StatusCode prv_tx(const GenericCan *can, const GenericCanMsg *msg) {
  GenericCanHw *gch = (GenericCanHw *)can;
  if (gch->base.interface != &s_interface) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanHw.");
  } else if (can == NULL || msg == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  return can_hw_transmit(msg->id, msg->extended, (uint8_t *)&msg->data, msg->dlc);
}

// register_rx
static StatusCode prv_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t mask,
                                  uint32_t filter, bool extended, void *context) {
  GenericCanHw *gch = (GenericCanHw *)can;
  if (gch->base.interface != &s_interface) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanHw.");
  } else if (can == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  status_ok_or_return(can_hw_add_filter(mask, filter, extended));
  return generic_can_helpers_register_rx(can, rx_handler, mask, filter, context, NULL);
}

StatusCode generic_can_hw_init(GenericCanHw *can_hw, const CanHwSettings *settings,
                               EventId fault_event) {
  s_interface.tx = prv_tx;
  s_interface.register_rx = prv_register_rx;

  memset(can_hw->base.rx_storage, 0, sizeof(can_hw->base.rx_storage));

  can_hw->base.interface = &s_interface;
  can_hw->fault_event = fault_event;
  status_ok_or_return(can_hw_init(settings));

  can_hw_register_callback(CAN_HW_EVENT_TX_READY, prv_tx_handler, can_hw);
  can_hw_register_callback(CAN_HW_EVENT_MSG_RX, prv_rx_handler, can_hw);
  can_hw_register_callback(CAN_HW_EVENT_BUS_ERROR, prv_bus_error_handler, can_hw);
  return STATUS_CODE_OK;
}
