#include "can.h"
#include "can_msg_defs.h"
#include "drive_can.h"
#include "generic_can_uart.h"
#include "gpio.h"
#include "heartbeat_rx.h"
#include "interrupt.h"
#include "mc_cfg.h"
#include "motor_controller.h"
#include "sequenced_relay.h"
#include "uart.h"
#include "wait.h"
#include "log.h"
#include "can_transmit.h"
#include "generic_can_mcp2515.h"
#include "gpio_it.h"
#include "log.h"
#include "soft_timer.h"
#include "debug_led.h"
#include "status.h"

typedef enum {
  MOTOR_EVENT_SYSTEM_CAN_RX = 0,
  MOTOR_EVENT_SYSTEM_CAN_TX,
  MOTOR_EVENT_SYSTEM_CAN_FAULT,
} MotorEvent;

static MotorControllerStorage s_controller_storage;
static GenericCanMcp2515 s_can_mcp2515;
static CanStorage s_can_storage;
static SequencedRelayStorage s_relay_storage;
static HeartbeatRxHandlerStorage s_powertrain_heartbeat;
static UartStorage s_uart_storage;

static RelayRxStorage s_relay_rx_storage; 

static void prv_setup_system_can(void) {
  CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,
    .bitrate = MC_CFG_CAN_BITRATE,
    .rx_event = MOTOR_EVENT_SYSTEM_CAN_RX,
    .tx_event = MOTOR_EVENT_SYSTEM_CAN_TX,
    .fault_event = MOTOR_EVENT_SYSTEM_CAN_FAULT,
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

static void prv_periodic_debug(SoftTimerId timer_id, void *context) {
  CanMessage msg = { 0 };
  can_pack_impl_u16(&msg, 0, SYSTEM_CAN_MESSAGE_MOTOR_DEBUG, 8, s_can_mcp2515.mcp2515->errors.eflg, s_can_mcp2515.mcp2515->errors.tec, s_can_mcp2515.mcp2515->errors.rec, 0);
  can_transmit(&msg, NULL);
  soft_timer_start_seconds(1, prv_periodic_debug, NULL, NULL);
}

// RX Handler for SYSTEM_CAN_MESSAGE_MOTOR_RELAY
static StatusCode prv_relay_rx(SystemCanMessage msg_id, uint8_t state, void *context) {
  GpioAddress *relay = context; 
  gpio_set_state(relay, state); 
  return STATUS_CODE_OK; 
}

static StatusCode prv_motor_relay_init(GpioAddress relay_pin) {
  // Initialize GPIO as output and state low 
  GpioSettings gpio_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
  };

  gpio_init_pin(&relay_pin, &gpio_settings);
  return relay_rx_configure_handler(&s_relay_rx_storage, SYSTEM_CAN_MESSAGE_MOTOR_RELAY,
                                    NUM_EE_RELAY_STATES, prv_relay_rx, &relay_pin);}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  prv_setup_system_can();
  prv_setup_motor_can();

  // clang-format off
  MotorControllerSettings mc_settings = {
    .motor_can = (GenericCan *)&s_can_mcp2515,
    .ids = {
      [MOTOR_CONTROLLER_LEFT] = {
          .motor_controller = MC_CFG_MOTOR_CAN_ID_MC_LEFT,
          .interface = MC_CFG_MOTOR_CAN_ID_DC_LEFT,
      },
      [MOTOR_CONTROLLER_RIGHT] = {
          .motor_controller = MC_CFG_MOTOR_CAN_ID_MC_RIGHT,
          .interface = MC_CFG_MOTOR_CAN_ID_DC_RIGHT,
      },
    },
    .max_bus_current = MC_CFG_MOTOR_MAX_BUS_CURRENT,
  };
  // clang-format on

  motor_controller_init(&s_controller_storage, &mc_settings);
  drive_can_init(&s_controller_storage);

  SequencedRelaySettings relay_settings = {
    .can_msg_id = SYSTEM_CAN_MESSAGE_MOTOR_RELAY,
    .left_relay = MC_CFG_RELAY_LEFT,
    .right_relay = MC_CFG_RELAY_RIGHT,
    .delay_ms = MC_CFG_RELAY_DELAY_MS,
  };
  GpioAddress relay = { .port = GPIO_PORT_A, .pin = 9 }; 
  prv_motor_relay_init(relay); 
  //sequenced_relay_init(&s_relay_storage, &relay_settings);

  heartbeat_rx_register_handler(&s_powertrain_heartbeat, SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT,
                                heartbeat_rx_auto_ack_handler, NULL);

  soft_timer_start_seconds(1, prv_periodic_debug, NULL, NULL);

  debug_led_init(DEBUG_LED_BLUE_A);
  debug_led_init(DEBUG_LED_GREEN);
  debug_led_init(DEBUG_LED_RED);
  if(!status_ok(soft_timer_start_seconds(15, mcp2515_watchdog, s_can_mcp2515.mcp2515, NULL))){
    LOG_DEBUG("Could not start MCP watchdog\n");
  }
  while (true) {
    Event e = { 0 };
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
    }

    mcp2515_poll(s_can_mcp2515.mcp2515);
  }

  return 0;
}