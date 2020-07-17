// Simple smoketest project for the solar boards
// specifically for the spv1020s

//
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "spi.h"
#include "mux.h"
#include "spv1020_mppt.h"

#define SPI_PORT SPI_PORT_2
// this is the mux output pin
// or the address of the spv1020, change it according to which spv1020 you want to read
#define SPV1020 0

#define SMOKETEST_WAIT_TIME_MS 1000

#define BAUDRATE 60000
#define MOSI_PIN \
  { .port = GPIO_PORT_B, 15 }
#define MISO_PIN \
  { .port = GPIO_PORT_B, 14 }
#define SCLK_PIN \
  { .port = GPIO_PORT_B, 13 }
#define CS_PIN \
  { .port = GPIO_PORT_B, 12 }

#define MUX_ENABLE                \
  {                               \
    .port = GPIO_PORT_B, .pin = 1 \
  }  // need to check hardward for correct pin
     // this pin is unused
#define MUX_OUTPUT                \
  {                               \
    .port = GPIO_PORT_B, .pin = 1 \
  }  // need to check hardward for correct pin
     // this pin is unused
#define SEL_PIN_0 \
  { .port = GPIO_PORT_B, .pin = 3 }
#define SEL_PIN_1 \
  { .port = GPIO_PORT_B, .pin = 4 }
#define SEL_PIN_2 \
  { .port = GPIO_PORT_B, .pin = 5 }

#define DISCONNECTED_MUX_OUTPUT 7

// will get the address and size of the demultiplexer
static MuxAddress s_mux_address = {
  .bit_width = 3,
  .sel_pins[0] = SEL_PIN_0,
  .sel_pins[1] = SEL_PIN_1,
  .sel_pins[2] = SEL_PIN_2,
  .mux_enable_pin = MUX_ENABLE,
  .mux_output_pin = MUX_OUTPUT,
};

static void prv_spv1020_check(SoftTimerId timer_id, void *context) {
  uint8_t status = 0xFF;      // 8th bit is set
  uint16_t pwm = 0xFFFF;      // over 900
  uint16_t vin = 0xFFFF;      // over 10 bits
  uint16_t current = 0xFFFF;  // over 10 bits

  mux_set(&s_mux_address, SPV1020);

  spv1020_turn_on(SPI_PORT);

  spv1020_read_status(SPI_PORT, &status);
  if (status != 0) {
    LOG_DEBUG("Reading Status failed");
  } else {
    LOG_DEBUG("SPV1020 #%d status is: 0x%x\r\n", SPV1020, status);
  }

  spv1020_read_pwm(SPI_PORT, &pwm);
  if (pwm == 0xFFFF) {
    LOG_DEBUG("Reading PWM failed");
  } else {
    LOG_DEBUG("SPV1020 #%d pwm is: 0x%x\r\n", SPV1020, pwm);
  }

  spv1020_read_voltage_in(SPI_PORT, &vin);
  if (vin == 0xFFFF) {
    LOG_DEBUG("Reading Voltage failed");
  } else {
    LOG_DEBUG("SPV1020 #%d voltage_in is: 0x%x\r\n", SPV1020, vin);
  }

  spv1020_read_current(SPI_PORT, &current);
  if (current == 0xFFFF) {
    LOG_DEBUG("Reading Current failed");
  } else {
    LOG_DEBUG("SPV1020 #%d current is: 0x%x\r\n", SPV1020, current);
  }

  spv1020_shut(SPI_PORT);

  status_ok_or_return(mux_set(&s_mux_address, DISCONNECTED_MUX_OUTPUT));

  soft_timer_start_millis(SMOKETEST_WAIT_TIME_MS, prv_spv1020_check, NULL, NULL);
}

int main(void) {
  event_queue_init();
  gpio_init();
  gpio_it_init();
  interrupt_init();
  soft_timer_init();

  SpiSettings spi_settings = {
    .baudrate = 60000,
    .mode = SPI_MODE_3,
    .mosi = MOSI_PIN,
    .miso = MISO_PIN,
    .sclk = SCLK_PIN,
    .cs = CS_PIN,
  };
  spi_init(SPI_PORT, &spi_settings);

  mux_init(&s_mux_address);

  LOG_DEBUG("Initializing spv1020 smoke test\n");

  Event e = { 0 };
  soft_timer_start_millis(SMOKETEST_WAIT_TIME_MS, prv_spv1020_check, NULL, NULL);
  while (true) {
    wait();
  }
  return 0;
}
