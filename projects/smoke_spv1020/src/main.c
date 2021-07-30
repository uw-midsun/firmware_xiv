// Simple smoketest project for the solar boards
// specifically for the spv1020s

// Periodically reads every SMOKETEST_WAIT_TIME_MS from ONE spv1020 on a solar board
// It will turn on the spv1020, then
// read the status, pwm, voltage_in, and current of the spv1020 respectively
// and output the all values on the screen
// then turn the specific spv1020 off

// this smoke test will still work even if only one SPV1020 is connected to the SPI GPIO pins

// Configurable items: mux output pin, wait time, spi port,
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "mux.h"
#include "soft_timer.h"
#include "spi.h"
#include "spv1020_mppt.h"
#include "wait.h"

#include <math.h>

#define SPI_PORT SPI_PORT_2

// these are the addresses of the spv1020s to read from
static uint8_t s_test_devices[] = { 0 };

// Set this to 1 if testing current sense, 0 otherwise
#define RELAY_ENABLE 1

#define RELAY_EN_PIN \
  { .port = GPIO_PORT_B, .pin = 4 }

// this is how often the reading will take place
// modify if you want to read more or less
#define SMOKETEST_WAIT_TIME_MS 500

#define BAUDRATE 6000000
#define MOSI_PIN \
  { .port = GPIO_PORT_B, 15 }
#define MISO_PIN \
  { .port = GPIO_PORT_B, 14 }
#define SCLK_PIN \
  { .port = GPIO_PORT_B, 13 }
#define CS_PIN \
  { .port = GPIO_PORT_B, 12 }

// these are for setting the demux inputs
#define SEL_PIN_0 \
  { .port = GPIO_PORT_B, .pin = 0 }
#define SEL_PIN_1 \
  { .port = GPIO_PORT_B, .pin = 1 }
#define SEL_PIN_2 \
  { .port = GPIO_PORT_B, .pin = 2 }

#define DISCONNECTED_MUX_OUTPUT 7

// this is the demux address
static MuxAddress s_mux_address = {
  .bit_width = 3,
  .sel_pins[0] = SEL_PIN_0,
  .sel_pins[1] = SEL_PIN_1,
  .sel_pins[2] = SEL_PIN_2,
};

// this checks the sepcific spv1020
static void prv_spv1020_check(SoftTimerId timer_id, void *context) {
  for (uint8_t addr_idx = 0; addr_idx < SIZEOF_ARRAY(s_test_devices); addr_idx++) {
    uint8_t address = s_test_devices[addr_idx];
    uint8_t status = 0xFF;      // 8th bit is set
    uint16_t pwm = 0xFFFF;      // over 900
    uint16_t vin = 0xFFFF;      // over 10 bits
    uint16_t current = 0xFFFF;  // over 10 bits

    mux_set(&s_mux_address, address);

    spv1020_turn_on(SPI_PORT);

    spv1020_read_status(SPI_PORT, &status);
    spv1020_read_pwm(SPI_PORT, &pwm);
    printf("........................\n");
    spv1020_read_voltage_in(SPI_PORT, &vin);
    spv1020_read_current(SPI_PORT, &current);
  
    if (status == 0xFF) {
      LOG_DEBUG("Reading Status failed\n");
    } else {
      LOG_DEBUG("SPV1020 #%d status is: 0x%x\n", address, status);
    }

    if (pwm == 0xFFFF) {
      LOG_DEBUG("Reading PWM failed\n");
    } else {
      LOG_DEBUG("SPV1020 #%d pwm is: 0x%x\n", address, pwm);
    }

    if (vin == 0xFFFF) {
      LOG_DEBUG("Reading Voltage failed\n");
    } else {
      int32_t scaled = (int32_t)(26.1f * (float)vin);
      LOG_DEBUG("SPV1020 #%d voltage_in is: 0x%x, scaled: %ld\n", address, vin, scaled);
    }
    

    if (current == 0xFFFF) {
      LOG_DEBUG("Reading Current failed\n");
    } else {
      float curf = (float)current;
      int32_t scaled = (int32_t)(-0.0249f * curf * curf + 30.285f * curf - 3315.3f);
      if (scaled < 0) {
        scaled = 0;
      }
      LOG_DEBUG("SPV1020 #%d current is: %d, scaled = %ld\n", address, current, scaled);
    }

    spv1020_shut(SPI_PORT);

    mux_set(&s_mux_address, DISCONNECTED_MUX_OUTPUT);
  }

  soft_timer_start_millis(SMOKETEST_WAIT_TIME_MS, prv_spv1020_check, NULL, NULL);
}

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  SpiSettings spi_settings = {
    .baudrate = BAUDRATE,
    .mode = SPI_MODE_3,
    .mosi = MOSI_PIN,
    .miso = MISO_PIN,
    .sclk = SCLK_PIN,
    .cs = CS_PIN,
  };
  spi_init(SPI_PORT, &spi_settings);

  mux_init(&s_mux_address);

  if (RELAY_ENABLE) {
    GpioAddress relay_en = RELAY_EN_PIN;
    GpioSettings relay_en_settings = {
      .direction = GPIO_DIR_OUT,        //
      .state = GPIO_STATE_HIGH,         //
      .resistor = GPIO_RES_NONE,        //
      .alt_function = GPIO_ALTFN_NONE,  //
    };
    gpio_init_pin(&relay_en, &relay_en_settings);
  }


  LOG_DEBUG("Initializing spv1020 smoke test\n");

  soft_timer_start_millis(SMOKETEST_WAIT_TIME_MS, prv_spv1020_check, NULL, NULL);
  while (true) {
    wait();
  }
  return 0;
}
