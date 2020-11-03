#include "controller_board_pins.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "mcp23008_gpio_expander.h"
#include "soft_timer.h"
#include "wait.h"

#define MCP23008_I2C_PORT I2C_PORT_2
#define MCP23008_I2C_ADDR 0x20

typedef enum {
  CENTRE_CONSOLE_BUTTON_POWER = 0,
  CENTRE_CONSOLE_BUTTON_PARKING,
  CENTRE_CONSOLE_BUTTON_HAZARDS,
  CENTRE_CONSOLE_BUTTON_DRIVE,
  CENTRE_CONSOLE_BUTTON_NEUTRAL,
  CENTRE_CONSOLE_BUTTON_REVERSE,
  NUM_CENTRE_CONSOLE_BUTTONS
} CentreConsoleButton;

typedef enum {
  CENTRE_CONSOLE_LED_BPS = 0,
  CENTRE_CONSOLE_LED_POWER,
  CENTRE_CONSOLE_LED_DRIVE,
  CENTRE_CONSOLE_LED_REVERSE,
  CENTRE_CONSOLE_LED_NEUTRAL,
  CENTRE_CONSOLE_LED_PARKING,
  CENTRE_CONSOLE_LED_HAZARDS,
  CENTRE_CONSOLE_LED_SPARE,
  NUM_CENTRE_CONSOLE_LEDS
} CentreConsoleLed;

static GpioAddress s_button_addresses[NUM_CENTRE_CONSOLE_BUTTONS] = {
  [CENTRE_CONSOLE_BUTTON_POWER] = { .port = GPIO_PORT_B, .pin = 0 },
  [CENTRE_CONSOLE_BUTTON_PARKING] = { .port = GPIO_PORT_A, .pin = 0 },
  [CENTRE_CONSOLE_BUTTON_HAZARDS] = { .port = GPIO_PORT_A, .pin = 1 },
  [CENTRE_CONSOLE_BUTTON_DRIVE] = { .port = GPIO_PORT_A, .pin = 6 },
  [CENTRE_CONSOLE_BUTTON_NEUTRAL] = { .port = GPIO_PORT_A, .pin = 5 },
  [CENTRE_CONSOLE_BUTTON_REVERSE] = { .port = GPIO_PORT_A, .pin = 7 },
};

static const Mcp23008GpioAddress s_led_addresses[NUM_CENTRE_CONSOLE_LEDS] = {
  [CENTRE_CONSOLE_LED_BPS] = { MCP23008_I2C_ADDR, 0 },
  [CENTRE_CONSOLE_LED_POWER] = { MCP23008_I2C_ADDR, 1 },
  [CENTRE_CONSOLE_LED_REVERSE] = { MCP23008_I2C_ADDR, 2 },
  [CENTRE_CONSOLE_LED_DRIVE] = { MCP23008_I2C_ADDR, 3 },
  [CENTRE_CONSOLE_LED_NEUTRAL] = { MCP23008_I2C_ADDR, 4 },
  [CENTRE_CONSOLE_LED_SPARE] = { MCP23008_I2C_ADDR, 5 },
  [CENTRE_CONSOLE_LED_PARKING] = { MCP23008_I2C_ADDR, 6 },
  [CENTRE_CONSOLE_LED_HAZARDS] = { MCP23008_I2C_ADDR, 7 },
};

static const char *s_button_name[NUM_CENTRE_CONSOLE_BUTTONS] = {
  [CENTRE_CONSOLE_BUTTON_POWER] = "power button",
  [CENTRE_CONSOLE_BUTTON_PARKING] = "parking button",
  [CENTRE_CONSOLE_BUTTON_HAZARDS] = "hazard button",
  [CENTRE_CONSOLE_BUTTON_DRIVE] = "drive button",
  [CENTRE_CONSOLE_BUTTON_NEUTRAL] = "neutral button",
  [CENTRE_CONSOLE_BUTTON_REVERSE] = "reverse button",
};

static const char *s_led_name[NUM_CENTRE_CONSOLE_LEDS] = {
  [CENTRE_CONSOLE_LED_REVERSE] = "reverse light", [CENTRE_CONSOLE_LED_DRIVE] = "drive light",
  [CENTRE_CONSOLE_LED_NEUTRAL] = "neutral light", [CENTRE_CONSOLE_LED_POWER] = "power light",
  [CENTRE_CONSOLE_LED_SPARE] = "spare light",     [CENTRE_CONSOLE_LED_PARKING] = "parking light",
  [CENTRE_CONSOLE_LED_HAZARDS] = "hazards light", [CENTRE_CONSOLE_LED_BPS] = "BPS light",
};

static void prv_button_int(const GpioAddress *address, void *context) {
  for (uint8_t i = 0; i < NUM_CENTRE_CONSOLE_BUTTONS; i++) {
    GpioAddress i_addr = s_button_addresses[i];
    if (i_addr.port == address->port && i_addr.pin == address->pin) {
      CentreConsoleButton butt = (CentreConsoleButton)i;
      GpioState state = NUM_GPIO_STATES;
      gpio_get_state(address, &state);
      LOG_DEBUG("=========\n");
      LOG_DEBUG("%s %d\n", s_button_name[butt], state);
      LOG_DEBUG("=========\n");
    }
  }
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  I2CSettings i2c_settings = {
    .scl = CONTROLLER_BOARD_ADDR_I2C2_SCL,
    .sda = CONTROLLER_BOARD_ADDR_I2C2_SDA,
    .speed = I2C_SPEED_FAST,
  };
  i2c_init(MCP23008_I2C_PORT, &i2c_settings);

  GpioSettings button_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  InterruptSettings button_interrupt_settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
  };
  for (CentreConsoleButton b = 0; b < NUM_CENTRE_CONSOLE_BUTTONS; b++) {
    gpio_init_pin(&s_button_addresses[b], &button_settings);
    gpio_it_register_interrupt(&s_button_addresses[b], &button_interrupt_settings,
                               INTERRUPT_EDGE_RISING_FALLING, prv_button_int, NULL);
  }

  mcp23008_gpio_init(MCP23008_I2C_PORT, MCP23008_I2C_ADDR);
  Mcp23008GpioSettings pin_settings = {
    .direction = MCP23008_GPIO_DIR_OUT,
    .state = MCP23008_GPIO_STATE_LOW,
  };
  for (CentreConsoleLed led = 0; led < NUM_CENTRE_CONSOLE_LEDS; led++) {
    mcp23008_gpio_init_pin(&s_led_addresses[led], &pin_settings);
  }

  // Turn everything on
  for (CentreConsoleLed i = 0; i < NUM_CENTRE_CONSOLE_LEDS; i++) {
      LOG_DEBUG("turning %s on\n", s_led_name[i]);
      mcp23008_gpio_set_state(&s_led_addresses[i], MCP23008_GPIO_STATE_HIGH);
      delay_ms(2000);
  }

  // Turn everything off
  for (CentreConsoleLed i = 0; i < NUM_CENTRE_CONSOLE_LEDS; i++) {
    LOG_DEBUG("turning %s off\n", s_led_name[i]);
    mcp23008_gpio_set_state(&s_led_addresses[i], MCP23008_GPIO_STATE_LOW);
    delay_ms(2000);
  }
  while (1) {
    wait();
  }
  return 0;
}
