#include "battery_voltage_monitor.h"
#include "can.h"
#include "can_unpack.h"

StatusCode prv_battery_voltage_monitor_rx(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
    // can unpack to voltage
    uint32_t voltage = 0;
    uint32_t current = 0;
    CAN_UNPACK_BATTERY_AGGREGATE_VC(msg, &voltage, &current);
    if (voltage >= CHARGER_BATTERY_VOLTAGE_THRESHOLD) {
        event_raise(CHARGER_STOP_EVENT_STOP, 0);
    }
}

StatusCode battery_voltage_monitor_init() {
    // register can rx
    can_register_rx_handler(SYSTEM_CAN_MESSAGE_BATTERY_AGGREGATE_VC, prv_battery_voltage_monitor_rx, NULL);
    return STATUS_CODE_OK;
}
