#include "fault_monitor.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "centre_console_events.h"
#include "centre_console_fault_reason.h"
#include "log.h"
#include "power_fsm.h"
#include "status.h"
#include "watchdog.h"

#define NO_FAULT 0

static WatchdogStorage s_watchdog_storage = { 0 };

static void prv_update_centre_console_status(StatusCode status) {
  if (status != STATUS_CODE_OK) {
    FaultReason fault = { .fields = { .area = EE_CONSOLE_FAULT_AREA_BPS_HEARTBEAT, .reason = 0 } };
    event_raise_priority(EVENT_PRIORITY_HIGHEST, CENTRE_CONSOLE_POWER_EVENT_FAULT, fault.raw);
  }
}

static void prv_watchdog_expiry(void *context) {
  LOG_DEBUG("bps heartbeat watchdog expiry\n");
  prv_update_centre_console_status(STATUS_CODE_INTERNAL_ERROR);
}

static StatusCode prv_rx_bps_heartbeat(const CanMessage *msg, void *context,
                                       CanAckStatus *ack_reply) {
  *ack_reply = CAN_ACK_STATUS_OK;
  uint8_t fault_bitset = 0;
  CAN_UNPACK_BPS_HEARTBEAT(msg, &fault_bitset);
  // faults on bps heartbeat fault, goes from fault to off
  prv_update_centre_console_status(fault_bitset == NO_FAULT ? STATUS_CODE_OK
                                                            : STATUS_CODE_INTERNAL_ERROR);
  watchdog_kick(&s_watchdog_storage);
  return STATUS_CODE_OK;
}

StatusCode fault_monitor_init(WatchdogTimeout timeout) {
  watchdog_start(&s_watchdog_storage, timeout, prv_watchdog_expiry, NULL);
  status_ok_or_return(can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT,
                                              prv_rx_bps_heartbeat, &s_watchdog_storage));
  return STATUS_CODE_OK;
}
