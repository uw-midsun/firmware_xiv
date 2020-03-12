#include "charger_controller.h"
#include "event_queue.h"
#include "generic_can.h"
#include "log.h"
#include "mcp2515.h"
#include "soft_timer.h"

#define CHARGER_PERIOD 1000
#define CCS_BMS_ID 0x1806E5F4
#define MAX_ALLOWABLE_VC 0x00000000ffffffff
// try and get the max allowable data
#define BCA_CCS_ID 0x18FF50E5

static SoftTimerId charger_controller_timer_id;

static GenericCan s_generic_can;
static GenericCanMsg s_gen_can_msg = {
  .id = CCS_BMS_ID,
  .extended = false,
  .data = MAX_ALLOWABLE_VC,
  .dlc = 0,
};

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  // send max allowable vc
  generic_can_tx(&s_generic_can, &s_gen_can_msg);

  soft_timer_start_millis(CHARGER_PERIOD, prv_timer_callback, context,
                          &charger_controller_timer_id);
}

void prv_rx_cb(const GenericCanMsg *msg, void *context) {
  LOG_DEBUG("RUNNING\n");

  if (msg->data & (uint64_t)(255 << 24)) {  // looking at fifth byte
    charger_controller_deactivate();
    // transmit max vc and not to charge
    GenericCanMsg can_msg = {
      .id = msg->id,
      .data = msg->data | (1 << 24),
      .dlc = msg->dlc,
      .extended = msg->extended,
    };
    generic_can_tx(&s_generic_can, &can_msg);
    if (msg->data & (uint64_t)(1 << 24)) {
      // hardware failure
    }
    if (msg->data & (uint64_t)(1 << 25)) {
      // temperature too high
    }
    if (msg->data & (uint64_t)(1 << 26)) {
      // input voltage wrong
    }
    if (msg->data & (uint64_t)(1 << 27)) {
      // communication failure
    }
  }
  // check each byte of the data
  // Byte 1 is at the end
  for (size_t i = 0; i < 7; i++) {
    if ((msg->data << (i * 8)) > (MAX_ALLOWABLE_VC << (i * 8))) {
      charger_controller_deactivate();
      // transmit max vc and not to charge
      // maybe raise event
      // cus vc extended max vc
      // mcp2515_tx(charger_data.storage, charger_data.id, charger_data.extended,
      // charger_data.data | (1 << 24), charger_data.dlc);
    }
  }
}

StatusCode charger_controller_init(GenericCan *generic_can) {
  s_generic_can = *generic_can;
  // register a rx
  generic_can_register_rx(generic_can, prv_rx_cb, GENERIC_CAN_EMPTY_MASK, BCA_CCS_ID, false, NULL);
  return charger_controller_activate();
}

StatusCode charger_controller_activate() {
  return soft_timer_start_millis(CHARGER_PERIOD, prv_timer_callback, NULL,
                                 &charger_controller_timer_id);
}

StatusCode charger_controller_deactivate() {
  bool cancel = soft_timer_cancel(charger_controller_timer_id);

  if (cancel) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_UNINITIALIZED;
  }
}