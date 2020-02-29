#include "mci_broadcast.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "test_helpers.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "generic_can.h"
#include "generic_can_mcp2515.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
#include "soft_timer.h"
#include "status.h"

#include "motor_can.h"
#include "wavesculptor.h"
#include "motor_controller.h"
#include "mci_events.h"

#define TEST_CAN_DEVICE_ID 12

typedef struct TestMciOutputStorage {
  MotorCanDriveCommand expected_value;
  bool pedal_sent;
} TestMciOutputStorage;

static MotorControllerStorage s_mci_storage;
static GenericCanMcp2515 s_can_mcp2515;
static CanStorage s_can_storage;
static TestMciOutputStorage s_test_mci_output_storage;

static void prv_setup_motor_can(void) {
  Mcp2515Settings mcp2515_settings = {
    .spi_port = SPI_PORT_2,
    .spi_baudrate = 6000000,
    .mosi = { .port = GPIO_PORT_B, 15 },
    .miso = { .port = GPIO_PORT_B, 14 },
    .sclk = { .port = GPIO_PORT_B, 13 },
    .cs = { .port = GPIO_PORT_B, 12 },
    .int_pin = { .port = GPIO_PORT_A, 8 },

    .can_bitrate = MCP2515_BITRATE_500KBPS,
    .loopback = true,
  };

  generic_can_mcp2515_init(&s_can_mcp2515, &mcp2515_settings);
}

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

static void prv_do_measurement_tx() {
  // Call mcp2515: storage->rx_cb(id.raw, extended, read_data, dlc, storage->context);
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  prv_setup_motor_can();
  prv_setup_system_can();
  s_mci_storage.motor_can = (GenericCan *)&s_can_mcp2515;
  mci_output_init(&s_mci_storage);
}

void teardown_test(void) {}

// lv - left velocity
// rv - right velocity
// lb - left bus measurement
// rb - right bus measurement

// Test 1: lb rv lv rb (check 2 output)

// Test 2: lb rb lv rv (check 2 output)

// Test 3: rb lb lv rv (check 2 output)

// Test 4: rb (check no output)

// Test 4: rb lb (check only 1 output)

// Test 5: rv lv (check only 1 output)

// Test 6: rb lv (check no output)

// Test 5: rv lv (check only 1 output) rb lb (check only 1 output)

// Can RX handler (knows which output to expect using counter and global storage)
// spoof Generic CAN TX (pass in message id and values)
MotorControllerBroadcastSettings broadcast_settings = {
  .motor_can = s_mci_storage.motor_can,
  .device_ids = {
    [LEFT_MOTOR_CONTROLLER] = MOTOR_CAN_ID_LEFT_MOTOR_CONTROLLER,
    [RIGHT_MOTOR_CONTROLLER] = MOTOR_CAN_ID_RIGHT_MOTOR_CONTROLLER,
  }
};
MotorControllerBroadcastStorage broadcast_storage;
mci_broadcast_init(&broadcast_storage, &broadcast_settings);

