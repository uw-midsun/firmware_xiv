#include "led_manager.h"

#include <stdbool.h>

#include "centre_console_events.h"
#include "event_queue.h"
#include "mcp23008_gpio_expander.h"
#include "status.h"

#define MCP23008_I2C_PORT I2C_PORT_2
#define MCP23008_I2C_ADDR 0x27

// todo(SOFT-296): get the list of leds, etc
static const Mcp23008GpioAddress s_led_to_address[NUM_CENTRE_CONSOLE_LEDS] = {
  [CENTRE_CONSOLE_LED_BPS] = { MCP23008_I2C_ADDR, 0 },
  [CENTRE_CONSOLE_LED_POWER] = { MCP23008_I2C_ADDR, 1 },
  [CENTRE_CONSOLE_LED_REVERSE] = { MCP23008_I2C_ADDR, 2 },
  [CENTRE_CONSOLE_LED_NEUTRAL] = { MCP23008_I2C_ADDR, 3 },
  [CENTRE_CONSOLE_LED_DRIVE] = { MCP23008_I2C_ADDR, 4 },
  [CENTRE_CONSOLE_LED_DRL] = { MCP23008_I2C_ADDR, 5 },
  [CENTRE_CONSOLE_LED_LOW_BEAM] = { MCP23008_I2C_ADDR, 6 },
  [CENTRE_CONSOLE_LED_HAZARDS] = { MCP23008_I2C_ADDR, 7 },
};

static const CentreConsoleLed s_mutually_exclusive_leds[] = {
  CENTRE_CONSOLE_LED_REVERSE, CENTRE_CONSOLE_LED_NEUTRAL, CENTRE_CONSOLE_LED_DRIVE,
  // parking?
};

StatusCode led_manager_init(void) {
  status_ok_or_return(mcp23008_gpio_init(MCP23008_I2C_PORT, MCP23008_I2C_ADDR));
  Mcp23008GpioSettings pin_settings = {
    .direction = MCP23008_GPIO_DIR_OUT,
    .state = MCP23008_GPIO_STATE_LOW,
  };
  for (CentreConsoleLed led = 0; led < NUM_CENTRE_CONSOLE_LEDS; led++) {
    status_ok_or_return(mcp23008_gpio_init_pin(&s_led_to_address[led], &pin_settings));
  }
  return STATUS_CODE_OK;
}

bool led_manager_process_event(Event *e) {
  if (e == NULL) return false;
}
