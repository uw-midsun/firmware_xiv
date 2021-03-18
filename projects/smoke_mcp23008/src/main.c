// Smoke test for mcp23008 gpio expander
// Tests every gpio pin
// Initializes all 8 pins, checks for correct initialization
// Periodically toggles states and logs comparison of pin with expected state

#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "mcp23008_gpio_expander.h"
#include "soft_timer.h"
#include "wait.h"

// wait time between gpio pin toggling
#define WAIT_TIME_MILLIS 1000

#define MCP23008_I2C_ADDRESS 0x20  // MCP23008 address
#define I2C_PORT I2C_PORT_2

#define I2C_SCL \
  { GPIO_PORT_B, 10 }
#define I2C_SDA \
  { GPIO_PORT_B, 11 }

void setup_i2c_and_mcp23008_gpio(void) {
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = I2C_SDA,
    .scl = I2C_SCL,
  };
  i2c_init(I2C_PORT, &i2c_settings);
  mcp23008_gpio_init(I2C_PORT, MCP23008_I2C_ADDRESS);
}

static StatusCode prv_mcp23008_init_all_pins(Mcp23008GpioDirection direction) {
  Mcp23008GpioSettings gpio_settings = {
    .direction = direction,
  };
  Mcp23008GpioAddress address = {
    .i2c_address = MCP23008_I2C_ADDRESS,
  };

  if (direction == MCP23008_GPIO_DIR_OUT) {
    gpio_settings.state = MCP23008_GPIO_STATE_HIGH;
  } else {
    gpio_settings.state = MCP23008_GPIO_STATE_LOW;
  }
  for (Mcp23008PinAddress pin = 0; pin < NUM_MCP23008_GPIO_PINS; pin++) {
    address.pin = pin;
    status_ok_or_return(mcp23008_gpio_init_pin(&address, &gpio_settings));
  }
  return STATUS_CODE_OK;
}

static StatusCode prv_mcp23008_check_pin_states(Mcp23008GpioState state) {
  Mcp23008GpioState in_state;
  Mcp23008GpioAddress address = { .i2c_address = MCP23008_I2C_ADDRESS };
  for (Mcp23008PinAddress pin = 0; pin < NUM_MCP23008_GPIO_PINS; pin++) {
    address.pin = pin;
    status_ok_or_return(mcp23008_gpio_get_state(&address, &in_state));
    if (state == in_state) {
      LOG_DEBUG("State for pin %d set and read correctly\n", pin);
    } else {
      LOG_DEBUG("State for pin %d incorrect\n", pin);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }
  return STATUS_CODE_OK;
}

static void prv_check_initial_states(void) {
  Mcp23008GpioState get_state;
  Mcp23008GpioAddress address = { .i2c_address = MCP23008_I2C_ADDRESS };

  for (Mcp23008PinAddress pin = 0; pin < NUM_MCP23008_GPIO_PINS; pin++) {
    address.pin = pin;
    mcp23008_gpio_get_state(&address, &get_state);
    LOG_DEBUG("State for PIN %d == %d\n", pin, get_state);
  }
}

static void prv_periodic_gpio_toggle_and_check(SoftTimerId timer_id, void *context) {
  Mcp23008GpioState *state = (Mcp23008GpioState *)context;
  Mcp23008GpioState get_state;
  Mcp23008GpioAddress address = { .i2c_address = MCP23008_I2C_ADDRESS };

  for (Mcp23008PinAddress pin = 0; pin < NUM_MCP23008_GPIO_PINS; pin++) {
    address.pin = pin;
    mcp23008_gpio_get_state(&address, &get_state);
    mcp23008_gpio_toggle_state(&address);
    mcp23008_gpio_get_state(&address, &get_state);
    if (get_state == *state) {
      LOG_DEBUG("State for pin %d set and read correctly\n", pin);
    } else {
      LOG_DEBUG("State for pin %d set and read incorrectly\n", pin);
    }
  }

  LOG_DEBUG("GPIO state = %s\n", *state == MCP23008_GPIO_STATE_HIGH ? "high" : "low");
  *state =
      (*state == MCP23008_GPIO_STATE_HIGH ? MCP23008_GPIO_STATE_LOW : MCP23008_GPIO_STATE_HIGH);

  soft_timer_start_millis(WAIT_TIME_MILLIS, prv_periodic_gpio_toggle_and_check, state, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  setup_i2c_and_mcp23008_gpio();

  LOG_DEBUG("Testing GPIO initialization...\n");
  prv_check_initial_states();
  LOG_DEBUG("Initializing all pins out...\n");
  prv_mcp23008_init_all_pins(MCP23008_GPIO_DIR_OUT);
  prv_mcp23008_check_pin_states(MCP23008_GPIO_STATE_HIGH);
  LOG_DEBUG("GPIO initialization complete. Now beginning toggling of GPIO states \n");

  Mcp23008GpioState state = MCP23008_GPIO_STATE_LOW;

  soft_timer_start_millis(100, prv_periodic_gpio_toggle_and_check, &state, NULL);

  while (true) {
    wait();
  }

  return 0;
}
