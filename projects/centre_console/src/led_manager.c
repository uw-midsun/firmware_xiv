#include "led_manager.h"

#include <stdbool.h>

#include "centre_console_events.h"
#include "event_queue.h"
#include "mcp23008_gpio_expander.h"
#include "status.h"

#define RETURN_IF_TRUE(x) \
  do {                    \
    if (x) return true;   \
  } while (0);

#define MCP23008_I2C_PORT I2C_PORT_2
#define MCP23008_I2C_ADDR 0x27

static const Mcp23008GpioAddress s_led_to_address[NUM_CENTRE_CONSOLE_LEDS] = {
  [CENTRE_CONSOLE_LED_BPS] = { MCP23008_I2C_ADDR, 0 },
  [CENTRE_CONSOLE_LED_POWER] = { MCP23008_I2C_ADDR, 1 },
  [CENTRE_CONSOLE_LED_REVERSE] = { MCP23008_I2C_ADDR, 2 },
  [CENTRE_CONSOLE_LED_NEUTRAL] = { MCP23008_I2C_ADDR, 3 },
  [CENTRE_CONSOLE_LED_DRIVE] = { MCP23008_I2C_ADDR, 4 },
  [CENTRE_CONSOLE_LED_FAULT] = { MCP23008_I2C_ADDR, 5 },
  [CENTRE_CONSOLE_LED_PARKING] = { MCP23008_I2C_ADDR, 6 },
  [CENTRE_CONSOLE_LED_HAZARDS] = { MCP23008_I2C_ADDR, 7 },
};

static void prv_set_led(CentreConsoleLed led, Mcp23008GpioState state) {
  mcp23008_gpio_set_state(&s_led_to_address[led], state);
}

static bool prv_try_handle_hazard_event(Event *e) {
  if (e->id == HAZARD_EVENT_ON || e->id == HAZARD_EVENT_OFF) {
    prv_set_led(CENTRE_CONSOLE_LED_HAZARDS,
                e->id == HAZARD_EVENT_ON ? MCP23008_GPIO_STATE_HIGH : MCP23008_GPIO_STATE_LOW);
    return true;
  }
  return false;
}

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
  RETURN_IF_TRUE(prv_try_handle_hazard_event(e));
  return false;
}
