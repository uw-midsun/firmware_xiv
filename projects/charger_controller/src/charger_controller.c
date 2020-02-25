#include "charger_controller.h"
#include "event_queue.h"
#include "log.h"
#include "mcp2515.h"
#include "soft_timer.h"

#define CHARGER_PERIOD 1000
#define CCS_BMS_ID 0x1806E5F4
uint64_t max_allowable_vc = 0;
// try and get the max allowable data
#define BCA_CCS_ID 0x18FF50E5

static SoftTimerId charger_controller_timer_id;
static SoftTimerId bms_received_timer_id;
ChargerData charger_data = { .id = CCS_BMS_ID, .extended = true, .data = 0, .dlc = 0 };

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  // send max allowable vc
  mcp2515_tx(charger_data.storage, charger_data.id, charger_data.extended, charger_data.data,
             charger_data.dlc);

  soft_timer_start_millis(CHARGER_PERIOD, prv_timer_callback, context, charger_controller_timer_id);
}

static void prv_rx_cb(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context) {
  if ((id == BCA_CCS_ID)) {
    charger_data.extended = extended;
    charger_data.data = data;
    charger_data.dlc = dlc;
    //check each byte of the data
    for (size_t i = 0; i < 7; i++) {
      if ((data << (i * 8)) > (max_allowable_vc << (i * 8))) {
        //transmit max vc and not to charge
        //cus vc extended max vc
        //mcp2515_tx(charger_data.storage, charger_data.id, charger_data.extended, charger_data.data | (1 << 24), charger_data.dlc);
      }
    }
    
    if (data & (255 << 24)) { //looking at fifth byte
      charger_controller_deactivate();
      //transmit max vc and not to charge
      //mcp2515_tx(charger_data.storage, charger_data.id, charger_data.extended, charger_data.data | (1 << 24), charger_data.dlc);
      if (data & (1 << 24)) {
        //hardware failure
      }
      if (data & (1 << 25)) {
        //temperature too high
      }
      if (data & (1 << 26)) {
        //input voltage wrong
      }
      if (data & (1 << 27)) {
        //communication failure
      }
    }
  }
}

StatusCode charger_controller_init(Mcp2515Storage *storage) {
  charger_data.storage = storage;
  // register a rx
  mcp2515_register_rx_cb(storage, prv_rx_cb, NULL);
  return charger_controller_activate();
}

StatusCode charger_controller_activate() {
  return soft_timer_start_millis(CHARGER_PERIOD, prv_timer_callback, NULL,
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