#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "adc.h"
#include "delay.h"  // For real-time delays
#include "gpio.h"   // General Purpose I/O control.
#include "gpio_it.h"
#include "interrupt.h"  // For enabling interrupts.
#include "log.h"
#include "mcp2515.h"
#include "misc.h"  // Various helper functions/macros.
#include "pwm_input.h"
#include "soft_timer.h"  // Software timers for scheduling future events.
#include "spi.h"

#define TOLERANCE (2)
#define PWM_READ_PERIOD_US 1000

typedef enum {
  EVSE_UNPLUGGED = 0,
  EVSE_PLUGGED_PRESSED,
  EVSE_PLUGGED_RELEASED,
  NUM_EVSE_STATES
} EvseState;

static Mcp2515Storage s_mcp_storage = { 0 };

EvseState get_evse_state(uint16_t adc_read) {
  if (2233 < adc_read && adc_read < 2631)
    return EVSE_UNPLUGGED;
  else if (754 < adc_read && adc_read < 1116)
    return EVSE_PLUGGED_RELEASED;
  else if (1459 < adc_read && adc_read < 1938)
    return EVSE_PLUGGED_PRESSED;
  else
    return NUM_EVSE_STATES;
}

void check_pwm_val(PwmInputReading *reading) {
  pwm_input_get_reading(PWM_TIMER_3, reading);
  LOG_DEBUG("read DC: %d | Period: %d, read period: %d\n", (int)reading->dc_percent,
            PWM_READ_PERIOD_US, (int)reading->period_us);
}

void check_adc_val(AdcChannel chan, uint16_t *data) {
  adc_read_converted(chan, data);
  LOG_DEBUG("ADC Reading: %d\n", *data);
  EvseState state = get_evse_state(*data);
  switch (state) {
    case EVSE_UNPLUGGED:
      LOG_DEBUG("evse unplugged\n");
      break;
    case EVSE_PLUGGED_PRESSED:
      LOG_DEBUG("evse plugged, button pressed\n");
      break;
    case EVSE_PLUGGED_RELEASED:
      LOG_DEBUG("evse plugged, button released\n");
      break;
    default:
      LOG_DEBUG("evse state unknown\n");
  }
}

static void rx_message_callback(uint32_t id, bool extended, uint64_t data, size_t dlc,
                                void *context) {
  LOG_DEBUG("message received:\nid: %lx\textended: %d\tdlc: %d\n", id, extended, dlc);
  // LOG_DEBUG("msb: %lx\n", (uint32_t) (data >> 32));
  // LOG_DEBUG("lsb: %lx\n", (uint32_t) (data & 0xffffffff));
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  // PWM stuff
  PwmTimer input_timer = PWM_TIMER_3;
  GpioAddress pwm_input = { .port = GPIO_PORT_A, .pin = 6 };
  GpioSettings input_settings = { .direction = GPIO_DIR_IN, .alt_function = GPIO_ALTFN_1 };
  gpio_init_pin(&pwm_input, &input_settings);
  pwm_input_init(input_timer, PWM_CHANNEL_1);
  PwmInputReading reading = { 0 };

  // ADC stuff
  GpioAddress pot_addr = { .port = GPIO_PORT_A, .pin = 7 };
  GpioSettings pot_settings = { .direction = GPIO_DIR_IN,
                                .state = GPIO_STATE_LOW,
                                .resistor = GPIO_RES_NONE,
                                .alt_function = GPIO_ALTFN_ANALOG };
  gpio_init_pin(&pot_addr, &pot_settings);
  adc_init(ADC_MODE_SINGLE);
  AdcChannel pot_channel = NUM_ADC_CHANNELS;
  adc_get_channel(pot_addr, &pot_channel);
  adc_set_channel(pot_channel, true);
  uint16_t pot_reading;

  Mcp2515Settings mcp_2515_spi_settings = { .spi_port = SPI_PORT_2,
                                            .baudrate = 6000000,
                                            .mosi = { .port = GPIO_PORT_B, .pin = 15 },
                                            .miso = { .port = GPIO_PORT_B, .pin = 14 },
                                            .sclk = { .port = GPIO_PORT_B, .pin = 13 },
                                            .cs = { .port = GPIO_PORT_B, .pin = 12 },
                                            .int_pin = { .port = GPIO_PORT_A, .pin = 8 },
                                            .loopback = false,
                                            .rx_cb = rx_message_callback };

  LOG_DEBUG("INITIALIZING_MCP2515\n");
  StatusCode s = mcp2515_init(&s_mcp_storage, &mcp_2515_spi_settings);
  if (s) {
    LOG_DEBUG("ERROR: status code not ok: %d\n", s);
  }
  // mcp2515_register_rx_cb(&s_mcp_storage, rx_message_callback, NULL);
  while (true) {
    // mcp2515_tx(&s_mcp_storage, 69, false, 0xDEADBEEF, 4);
    // check_pwm_val(&reading);
    check_adc_val(pot_channel, &pot_reading);
    delay_ms(500);
  }

  return 0;
}
