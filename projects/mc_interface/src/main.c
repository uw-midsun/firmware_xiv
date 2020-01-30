#include "can.h"
#include "generic_can_mcp2515.h"
#include "status.h"
#include "interrupt.h"
#include "gpio.h"
#include "gpio_it.h"

#include "precharge_control.h"
#include "drive_rx.h"
#include "drive_output.h"
#include "drive_fsm.h"

static MotorControllerStorage s_controller_storage;
static GenericCanMcp2515 s_can_mcp2515;
static CanStorage s_can_storage;

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

int main(void) {
    interrupt_init();
    gpio_init();
    gpio_it_init();
    soft_timer_init();
    event_queue_init();
    prv_setup_system_can();
    prv_setup_motor_can();

    //TODO: dependent on mcp2515 driver improvements, may need to add
          //code to add filters for the messages we want

    //TODO: write motor_controller.h or similar with definition
          //of MotorControllerStorage object

    //TODO: init periodic tx to motor controller here

    //TODO: setup periodic can broadcast (write data_broadcast.c)

    //TODO: setup heartbeat rx and ack

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