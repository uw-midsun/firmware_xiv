#include "can.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "delay.h"
#include "exported_enums.h"
#include "test_helpers.h"

#include "motor_controller.h"
#include "precharge_control.h"

static CanStorage s_can_storage;
static MotorControllerStorage s_mci_storage;

void prv_setup_system_can() {
    CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = MCI_CAN_EVENT_RX,
    .tx_event = MCI_CAN_EVENT_TX,
    .fault_event = MCI_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_init(&s_can_storage, &can_settings);
}

void prv_mci_storage_init(void *context) {
    MotorControllerStorage *storage = context;
    MotorControllerSettings settings = {
        .precharge_control = { .port = GPIO_PORT_A, .pin = 9 },
        .precharge_control2 = { .port = GPIO_PORT_B, .pin = 1 },
        .precharge_monitor = { .port = GPIO_PORT_B, .pin = 0 }
    };
    storage->settings = settings;
    storage->precharge_state = MCI_PRECHARGE_DISCHARGED;
}

void setup_test(void) {
    prv_setup_system_can();
    prv_mci_storage_init(&s_mci_storage);
    TEST_ASSERT(precharge_control_init(&s_mci_storage) == STATUS_CODE_OK);
}

void teardown_test(void) {}

void test_run(void) {
    MotorControllerStorage *storage = &s_mci_storage;
    TEST_ASSERT_TRUE(storage->precharge_state == MCI_PRECHARGE_DISCHARGED);
    CanAckStatus ack_status = CAN_ACK_STATUS_INVALID;
    CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_status, EE_POWER_MAIN_SEQUENCE_BEGIN_PRECHARGE);
    // TODO: Check if this is valid amount of time to wait, i.e. how long does precharge take
    delay_ms(500);
    TEST_ASSERT_TRUE(ack_status == CAN_ACK_STATUS_OK);
    TEST_ASSERT_TRUE(storage->precharge_state == MCI_PRECHARGE_CHARGED);
    // TODO: add a check to ensure proper discharge
}