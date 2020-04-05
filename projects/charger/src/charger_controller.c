#include "charger_controller.h"

#include "can_transmit.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "generic_can.h"
#include "generic_can_mcp2515.h"
#include "log.h"
#include "mcp2515.h"
#include "soft_timer.h"

#define CHARGER_PERIOD 1000
#define CCS_BMS_ID 0x1806E5F4
#define MAX_ALLOWABLE_VC 0xffff

// you need to find max current from the pwm from the control pilot
// signal.
// control_pilot pwm has some duty cycle, that dc proportional to max
// current. YOU CAN FIND THIS IN THE Charging Standard.
// you CAN hardcode max voltage, it can be
// something near when our battery is getting fully charged.
// Ask Micah Black what that voltage is.
// what is the voltage of our batteries when they're 95% charged.

#define BCA_CCS_ID 0x18FF50E5

static SoftTimerId charger_controller_timer_id;

static GenericCan *s_generic_can;
static GenericCanMsg s_gen_can_msg = {
  .id = CCS_BMS_ID,
  .extended = true,
  .data = MAX_ALLOWABLE_VC,
  .dlc = 8,
};

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  // send max allowable vc
  generic_can_tx(s_generic_can, &s_gen_can_msg);

  soft_timer_start_millis(CHARGER_PERIOD, prv_timer_callback, context,
                          &charger_controller_timer_id);
}

void prv_generic_rx_callback(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context) {
  // still need to test if this receives
  LOG_DEBUG("RUNNING\n");
  GenericCanMsg can_msg = {
    .id = CCS_BMS_ID,
    .data = MAX_ALLOWABLE_VC | (1 << 24),
    .dlc = 8,
    .extended = true,
  };

  if (id == BCA_CCS_ID) {
    if (data & (uint64_t)(255 << 24)) {  // looking at fifth byte
      charger_controller_deactivate();
      // transmit max vc and not to charge
      generic_can_tx(s_generic_can, &can_msg);
      if (data & (uint64_t)(1 << 24)) {
        // hardware failure
        CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_HARDWARE_FAULT);
      }
      if (data & (uint64_t)(1 << 25)) {
        // temperature too high
        CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_TEMP_FAULT);
      }
      if (data & (uint64_t)(1 << 26)) {
        // input voltage wrong
        CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_INPUT_FAULT);
      }
      if (data & (uint64_t)(1 << 27)) {
        // wrong state
        CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_STATE_FAULT);
      }
      if (data & (uint64_t)(1 << 28)) {
        // communication fault
        CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_COMMUNICATION_FAULT);
      }
    }
    // probably don't need this
    // check each byte of the data if over max allowable vc
    // Byte 1 is at the end
    // for (size_t i = 4; i < 7; ++i) {
    //   if ((data << (i * 8)) > (MAX_ALLOWABLE_VC << (i * 8))) {
    //     charger_controller_deactivate();
    //     // transmit max vc and not to charge
    //     generic_can_tx(&s_generic_can, &can_msg);
    //     // maybe raise event
    //     // cus vc extended max vc
    //     break;
    //   }
    // }
  }
}

// needs generic_can_mcp2515_init
StatusCode charger_controller_init(GenericCanMcp2515 *can_mcp2515) {
  s_generic_can = (GenericCan *)can_mcp2515;
  // register a rx
  return mcp2515_register_cbs(can_mcp2515->mcp2515, prv_generic_rx_callback, NULL, NULL);
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