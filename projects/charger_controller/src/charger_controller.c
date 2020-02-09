#include "charger_controller.h"
#include "charger_events.h"
#include "event_queue.h"
#include "log.h"
#include "soft_timer.h"

#define CHARGER_PERIOD 1000

static SoftTimerId charger_controller_timer_id;

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  ChargerData *data = context;
  mcp2515_tx(data->storage, data->id, data->extended, data->data, data->dlc);

  soft_timer_start_millis(CHARGER_PERIOD, prv_timer_callback, context, charger_controller_timer_id);
}

static void prv_rx_cb(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context) {
  // should deactivate timer???
  // need to check these values
  if ((id == 0) && (!extended) && (data == 0) && (dlc == 0)) {
    charger_controller_deactivate();
  }
}

StatusCode charger_controller_init(ChargerData *data) {
  // register a rx
  mcp2515_register_rx_cb(data->storage, prv_rx_cb, NULL);
  return charger_controller_activate(data);
}

StatusCode charger_controller_activate(ChargerData *data) {
  return soft_timer_start_millis(CHARGER_PERIOD, prv_timer_callback, data,
                                 charger_controller_timer_id);
}

StatusCode charger_controller_deactivate() {
  bool cancel = soft_timer_cancel(charger_controller_timer_id);

  if (cancel) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_UNINITIALIZED;
  }
}