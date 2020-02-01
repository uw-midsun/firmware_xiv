#include <string.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_hw.h"
#include "generic_can_mcp2515.h"
#include "gpio.h"
#include "gpio_it.h"
#include "heartbeat_rx.h"
#include "interrupt.h"
#include "status.h"

#include "drive_fsm.h"
#include "drive_output.h"
#include "drive_rx.h"
#include "motor_controller.h"
#include "precharge_control.h"
#include "mci_broadcast.h"

static MotorControllerStorage s_controller_storage;
static GenericCanMcp2515 s_can_mcp2515;
static CanStorage s_can_storage;

static HeartbeatRxHandlerStorage s_powertrain_heartbeat;

static void prv_setup_system_can(void) {
  CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = MOTOR_EVENT_SYSTEM_CAN_RX,
    .tx_event = MOTOR_EVENT_SYSTEM_CAN_TX,
    .fault_event = MOTOR_EVENT_SYSTEM_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };
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
  };

  generic_can_mcp2515_init(&s_can_mcp2515, &mcp2515_settings);
}

static void prv_setup_controller_storage(void) {
  memset(&s_controller_storage, 0, sizeof(s_controller_storage));
  MotorControllerSettings *controller_settings = &(s_controller_storage.settings);
  controller_settings->motor_can =  (GenericCan *)&s_can_mcp2515;
  controller_settings->motor_controller_ids[MOTOR_CONTROLLER_LEFT] = MOTOR_CAN_ID_MC_LEFT;
  controller_settings->motor_controller_ids[MOTOR_CONTROLLER_RIGHT] = MOTOR_CAN_ID_MC_RIGHT;
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  prv_setup_system_can();
  prv_setup_motor_can();
  prv_setup_controller_storage();

  drive_rx_init(&s_controller_storage);

  drive_fsm_init(&s_controller_storage);

  precharge_init(&s_controller_storage);

  drive_output_init(&s_controller_storage);

  mci_broadcast_init(&s_controller_storage);

  // TODO(SOFT-40): dependent on mcp2515 driver improvements, may need to add
  // code to add filters for the messages we want

  // TODO(SOFT-40): implement watchdog counter (increment upon tx, reset upon recieve throttle)
        // event_raise neutral if over 15 or something

  heartbeat_rx_register_handler(&s_powertrain_heartbeat, SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT,
                                heartbeat_rx_auto_ack_handler, NULL);

  while (true) {
    Event e = { 0 };
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
      precharge_fsm_process_event(&e);
      drive_fsm_process_event(&e);
    }
  }

  return 0;
}
