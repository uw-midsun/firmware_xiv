#include "can.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "generic_can_mcp2515.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "pedal_rx.h"
#include "soft_timer.h"

#include "cruise_rx.h"
#include "drive_fsm.h"
#include "mci_broadcast.h"
#include "mci_events.h"
#include "mci_output.h"
#include "motor_can.h"
#include "motor_controller.h"
#include "precharge_control.h"

#include "log.h"

static CanStorage s_can_storage;
static GenericCanMcp2515 s_can_mcp2515;
static MotorControllerStorage s_mci_storage;

static Mcp2515Storage s_test_can_mcp2515;

// For alternating filter IDs
static bool s_filter_id = false;

static void prv_change_filter(void) {
  // Testing with voltage vector measurements instead of speed since they give values
  LOG_DEBUG("Changing filter ID to %d\n", (s_filter_id ? 0x405 : 0x402));
  uint32_t filters[2];
  filters[0] = 0x1;
  if(!s_filter_id) {
    filters[1] = 0x402;
  } else {
    filters[1] = 0x405;
  }

  LOG_DEBUG("Change filter result %d\n", mcp2515_set_filter(&s_test_can_mcp2515, filters));
}

// CB for rx received
static void prv_test_receive_rx(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context) {
  LOG_DEBUG("received rx from id: %d\n", (int)id);
  LOG_DEBUG("Data: 0x%x\n", (int)data);
  // Change to other filter and re-init
  s_filter_id ^= 1;
  prv_change_filter();
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
    .loopback = false,
  };

  can_init(&s_can_storage, &can_settings);
  drive_fsm_init();
  cruise_rx_init();
}


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
    .loopback = false,
    // filter at MCP2515 instead of through SW
    .filters = {
      [MCP2515_FILTER_ID_RXF0] = {.raw = 0x1},
      [MCP2515_FILTER_ID_RXF1] = {.raw = 0x402},
    },
  };
  // debug stuff to apply filters directly to mcp2515
  //generic_can_mcp2515_init(&s_can_mcp2515, &mcp2515_settings);
  mcp2515_init(&s_test_can_mcp2515, &mcp2515_settings);
  mcp2515_register_cbs(&s_test_can_mcp2515, prv_test_receive_rx, NULL, NULL);
}



void prv_mci_storage_init(void *context) {
  PrechargeControlSettings precharge_settings = {
    .precharge_control = { .port = GPIO_PORT_A, .pin = 9 },
    .precharge_monitor = { .port = GPIO_PORT_B, .pin = 0 },
    .precharge_monitor2 = { .port = GPIO_PORT_A, .pin = 10 },
  };
  precharge_control_init(&precharge_settings);

  MotorControllerBroadcastSettings broadcast_settings =
      { .motor_can = (GenericCan *)&s_can_mcp2515,
        .device_ids = {
            [LEFT_MOTOR_CONTROLLER] = MOTOR_CAN_ID_LEFT_MOTOR_CONTROLLER,
            [RIGHT_MOTOR_CONTROLLER] = MOTOR_CAN_ID_RIGHT_MOTOR_CONTROLLER,
        } };
  mci_broadcast_init(&s_mci_storage.broadcast_storage, &broadcast_settings);

  mci_output_init(&s_mci_storage.mci_output_storage, (GenericCan *)&s_can_mcp2515);
}

int main(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  //prv_setup_system_can();
  prv_setup_motor_can();

  //prv_test_change_filter();
  //prv_mci_storage_init(&s_mci_storage);
  //drive_fsm_init();

  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
      //can_process_event(&e);
    }
  }

  return 0;
}
