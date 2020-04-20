#include "fault_monitor.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "status.h"
#include "watchdog.h"

#define NO_FAULT 0

static FaultStatus s_car_status = NUM_FAULT_STATUS;

typedef struct FaultMonitorStorage {
  uint16_t bps_fault_bitset;
  WatchdogStorage watchdog_storage;
} FaultMonitorStorage;

static FaultMonitorStorage s_storage = { 0 };

static void prv_update_centre_console_status() {
  s_car_status = (s_storage.bps_fault_bitset == NO_FAULT) ? FAULT_STATUS_OK : FAULT_STATUS_FAULT;
}

static void prv_watchdog_expiry(void *context) {
  s_car_status = FAULT_STATUS_ACK_FAULT;
}

static StatusCode prv_rx_bps_heartbeat(const CanMessage *msg, void *context,
                                       CanAckStatus *ack_reply) {
  *ack_reply = CAN_ACK_STATUS_OK;
  FaultMonitorStorage *storage = (FaultMonitorStorage *)context;
  uint8_t fault_bitset = 0;
  CAN_UNPACK_BPS_HEARTBEAT(msg, &fault_bitset);
  s_storage.bps_fault_bitset = fault_bitset;
  prv_update_centre_console_status();
  watchdog_kick(&storage->watchdog_storage);
  return STATUS_CODE_OK;
}

StatusCode fault_monitor_init(WatchdogTimeout timeout) {
  watchdog_start(&(s_storage.watchdog_storage), timeout, prv_watchdog_expiry,
                 &s_storage);
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, prv_rx_bps_heartbeat, &s_storage));
  return STATUS_CODE_OK;
}

FaultStatus *get_fault_status(void) {
  return &s_car_status;
}
