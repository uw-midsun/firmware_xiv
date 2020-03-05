#include "can.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "generic_can_mcp2515.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "pedal_rx.h"
#include "soft_timer.h"

#include "drive_fsm.h"
#include "mci_broadcast.h"
#include "mci_events.h"
#include "mci_output.h"
#include "motor_can.h"
#include "motor_controller.h"
#include "precharge_control.h"

static CanStorage s_can_storage;
static GenericCanMcp2515 s_can_mcp2515;
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
    .loopback = false,
  };

  can_init(&s_can_storage, &can_settings);
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

void prv_mci_storage_init(void *context) {
  PrechargeControlSettings precharge_settings = {
    .precharge_control = { .port = GPIO_PORT_A, .pin = 9 },
    .precharge_control2 = { .port = GPIO_PORT_B, .pin = 1 },
    .precharge_monitor = { .port = GPIO_PORT_B, .pin = 0 }
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
  gpio_init();
  gpio_it_init();
  interrupt_init();
  soft_timer_init();

  prv_setup_system_can();
  prv_setup_motor_can();

  prv_mci_storage_init(&s_mci_storage);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
      can_process_event(&e);
    }
  }

  return 0;
}
