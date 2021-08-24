// This project will receive a CAN message
// on the stm32 and perform specific callback
// based on the message

// To run this project, simply run
// make babydriver
// and that should program to the stm32 / run on x86
// To send a CAN message from Python write
// can_util.send_message(<id>, <data>) to send can message

#include "can_transmit.h"
#include "debug_led.h"

#include "adc.h"
#include "adc_read.h"
#include "can.h"
#include "can_hook.h"
#include "can_msg_defs.h"
#include "can_uart.h"
#include "controller_board_pins.h"
#include "dispatcher.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_get.h"
#include "gpio_interrupts.h"
#include "gpio_it.h"
#include "gpio_set.h"
#include "i2c_read.h"
#include "i2c_write.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "spi_exchange.h"
#include "uart.h"
#include "wait.h"

typedef enum {
  CAN_EVENT_RX = 0,
  CAN_EVENT_TX,
  CAN_EVENT_FAULT,
  NUM_CAN_EVENTS,
} CanEvent;

static CanStorage s_can_storage;
static CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_BABYDRIVER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = CAN_EVENT_RX,
  .tx_event = CAN_EVENT_TX,
  .fault_event = CAN_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

static UartStorage s_uart;
static CanUart s_can_uart = {
  .uart = UART_PORT_1,
};

int main() {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  event_queue_init();
  soft_timer_init();
  adc_init(ADC_MODE_SINGLE);

  can_init(&s_can_storage, &s_can_settings);

  debug_led_init(DEBUG_LED_BLUE_A);
  debug_led_init(DEBUG_LED_BLUE_B);
  debug_led_init(DEBUG_LED_GREEN);
  debug_led_init(DEBUG_LED_RED);

// #ifndef UARTLOGS
  // Enable CAN-over-UART for interacting with babydriver without a CAN adapter
  UartSettings uart_settings = {
    .baudrate = 115200,
    .tx = CONTROLLER_BOARD_ADDR_DEBUG_USART1_TX,
    .rx = CONTROLLER_BOARD_ADDR_DEBUG_USART1_RX,
    .alt_fn = GPIO_ALTFN_0,  // see https://www.st.com/resource/en/datasheet/stm32f072cb.pdf, tbl 16
  };
  uart_init(UART_PORT_1, &uart_settings, &s_uart);
  can_hook_init(&s_can_storage);
  can_uart_init(&s_can_uart);
  can_uart_enable_auto_tx(&s_can_uart);
  can_uart_enable_auto_rx(&s_can_uart);
// #endif

  // debug_led_set_state(DEBUG_LED_BLUE_A, true);
  // debug_led_set_state(DEBUG_LED_BLUE_B, true);
  // debug_led_set_state(DEBUG_LED_RED, true);
  // debug_led_set_state(DEBUG_LED_GREEN, true);

  dispatcher_init();
  adc_read_init();
  gpio_set_init();
  gpio_get_init();
  gpio_interrupts_init();
  i2c_write_init(I2C_WRITE_DEFAULT_TIMEOUT_MS);
  i2c_read_init(I2C_READ_DEFAULT_TX_DELAY_MS);
  spi_exchange_init(DEFAULT_SPI_EXCHANGE_TIMEOUT_MS, DEFAULT_SPI_EXCHANGE_TX_DELAY);

  LOG_DEBUG("Welcome to BabyDriver!\n");
  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_STATUS, 0xAA, 0, 0, 0, 0, 0, 0);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
    wait();
  }

  return 0;
}
