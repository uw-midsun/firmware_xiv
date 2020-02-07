#include "drive_rx.h"

#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_mcu.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_DRIVE_RX_CAN_DEVICE_ID 12
#define TEST_DRIVE_RX_TX_PERIOD_MS 100

typedef enum {
  TEST_DRIVE_RX_CAN_RX = 10,
  TEST_DRIVE_RX_CAN_TX,
  TEST_DRIVE_RX_CAN_FAULT,
} TestDriveCanEvent;

typedef enum {
  TEST_DRIVE_RX_DRIVE_EVENT = 0,
  TEST_DRIVE_RX_NEUTRAL_EVENT,
  TEST_DRIVE_RX_REVERSE_EVENT,
  NUM_TEST_DRIVE_RX_EVENT
} TestDriveRxEvent;

typedef struct {
  DriveRxStorage *dr_storage;
  EEDriveState expected;
  uint8_t curr_tx_test;
  bool handled_last;
  bool completed_tx;
  bool completed_all;
} TestDriveRxStorage;

static CanStorage s_can_storage;
static EEDriveState s_test_drive_values[] = {
  [0] = EE_DRIVE_STATE_NEUTRAL, [1] = EE_DRIVE_STATE_DRIVE,   [2] = EE_DRIVE_STATE_NEUTRAL,
  [3] = EE_DRIVE_STATE_REVERSE, [4] = EE_DRIVE_STATE_NEUTRAL, [5] = EE_DRIVE_STATE_DRIVE,
  [6] = EE_DRIVE_STATE_NEUTRAL,
};
static EEDriveState s_drive_state_lookup[NUM_TEST_DRIVE_RX_EVENT] = {
  [TEST_DRIVE_RX_NEUTRAL_EVENT] = EE_DRIVE_STATE_NEUTRAL,
  [TEST_DRIVE_RX_DRIVE_EVENT] = EE_DRIVE_STATE_DRIVE,
  [TEST_DRIVE_RX_REVERSE_EVENT] = EE_DRIVE_STATE_REVERSE,
};

static void prv_test_drive_rx_process_event(TestDriveRxStorage *storage, Event *event) {
  EEDriveState drive_state = drive_rx_get_state(storage->dr_storage);
  if (event->id > NUM_TEST_DRIVE_RX_EVENT) return;
  TEST_ASSERT_EQUAL(storage->expected, s_drive_state_lookup[event->id]);
  TEST_ASSERT_EQUAL(storage->expected, drive_state);
  storage->handled_last = true;
  storage->completed_all = storage->handled_last && storage->completed_tx;
}

static void prv_transmit_test_pedal_values(SoftTimerId timer_id, void *context) {
  TestDriveRxStorage *storage = context;
  TEST_ASSERT_TRUE(storage->handled_last);
  size_t i = storage->curr_tx_test++;
  storage->handled_last = false;
  CAN_TRANSMIT_DRIVE_STATE(s_test_drive_values[i]);
  storage->expected = s_test_drive_values[i];
  if (storage->curr_tx_test == SIZEOF_ARRAY(s_test_drive_values)) {
    storage->completed_tx = true;
  } else {
    soft_timer_start_millis(TEST_DRIVE_RX_TX_PERIOD_MS, prv_transmit_test_pedal_values, storage,
                            NULL);
  }
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = TEST_DRIVE_RX_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = TEST_DRIVE_RX_CAN_RX,
    .tx_event = TEST_DRIVE_RX_CAN_TX,
    .fault_event = TEST_DRIVE_RX_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));
}

void teardown_test(void) {}

void test_drive_rx(void) {
  DriveRxSettings settings = {
    .drive_event = TEST_DRIVE_RX_DRIVE_EVENT,
    .neutral_event = TEST_DRIVE_RX_NEUTRAL_EVENT,
    .reverse_event = TEST_DRIVE_RX_REVERSE_EVENT,
  };

  DriveRxStorage dr_storage = { 0 };

  TestDriveRxStorage storage = {
    .dr_storage = &dr_storage,
    .expected = NUM_EE_DRIVE_STATES,
    .curr_tx_test = 0,
    .handled_last = true,
    .completed_tx = false,
    .completed_all = false,
  };

  TEST_ASSERT_OK(drive_rx_init(&dr_storage, &settings));

  soft_timer_start_millis(TEST_DRIVE_RX_TX_PERIOD_MS, prv_transmit_test_pedal_values, &storage,
                          NULL);
  while (!storage.completed_all) {
    Event e = { 0 };
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
      prv_test_drive_rx_process_event(&storage, &e);
    }
  }
}
