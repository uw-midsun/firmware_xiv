// This project will receive a CAN message
// on the stm32 and perform specific callback
// based on the message

// To run this project, simply run
// make babydriver
// and that should program to the stm32 / run on x86
// To send a CAN message from Python write
// can_util.send_message(<id>, <data>) to send can message

#include "adc.h"
#include "adc_read.h"
#include "can.h"
#include "can_msg_defs.h"
#include "dispatcher.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_get.h"
#include "gpio_interrupts.h"
#include "gpio_it.h"
#include "gpio_set.h"
#include "i2c_write.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "spi_exchange.h"
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

int main() {
  LOG_DEBUG("Welcome to BabyDriver!\n");
  gpio_init();
  gpio_it_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  adc_init(ADC_MODE_SINGLE);

  can_init(&s_can_storage, &s_can_settings);

  dispatcher_init();
  adc_read_init();
  gpio_set_init();
  gpio_get_init();
  gpio_interrupts_init();
  i2c_write_init(I2C_WRITE_DEFAULT_TIMEOUT_MS);
  spi_exchange_init(DEFAULT_SPI_EXCHANGE_TIMEOUT_MS, DEFAULT_SPI_EXCHANGE_TX_DELAY);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
    wait();
  }

  return 0;
}
