// Smoke test for pca9539r gpio expander
// Checks all pins configure to inputs/outputs, then toggles gpio states of all 16 pins
// At every stage, prints comparison of configuration registers against expected results
// I2C settings may need to be configured to match wiring on board
// Changing WAIT_TIME_MILLIS allows different times between toggling

#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "pca9539r_gpio_expander.h"
#include "soft_timer.h"
#include "wait.h"

// used to adjust time between gpio pin toggling
#define WAIT_TIME_MILLIS 1000

#define PCA9539_I2C_ADDRESS 0x74  // PCA9539 address
#define I2C_PORT I2C_PORT_2
// I2C_PORT_1 has SDA on PB9 and SCL on PB8
// I2C_PORT_2 has SDA on PB11 and SCL on PB10

#define PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

void setup_test(void) {
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = PIN_I2C_SDA,
    .scl = PIN_I2C_SCL,
  };
  i2c_init(I2C_PORT, &i2c_settings);
  pca9539r_gpio_init(I2C_PORT, PCA9539_I2C_ADDRESS);
}

// initialize all pins to in/out - must be called after pca9539r_gpio_init
StatusCode pca9539r_init_all_pins(Pca9539rGpioDirection direction) {
  Pca9539rGpioSettings gpio_settings = {
    .direction = direction,
  };
  Pca9539rGpioAddress address = {
    .i2c_address = PCA9539_I2C_ADDRESS,
  };

  if (direction == PCA9539R_GPIO_DIR_OUT) {
    gpio_settings.state = PCA9539R_GPIO_STATE_HIGH;
  } else {
    gpio_settings.state = PCA9539R_GPIO_STATE_LOW;
  }
  for (Pca9539rPinAddress pin = PCA9539R_PIN_IO0_0; pin < NUM_PCA9539R_GPIO_PINS; pin++) {
    address.pin = pin;
    status_ok_or_return(pca9539r_gpio_init_pin(&address, &gpio_settings));
  }
  return STATUS_CODE_OK;
}

// reads back configuration registers and compares them to input state
StatusCode pca9539r_check_all_pin_states(Pca9539rGpioState state) {
  Pca9539rGpioState in_state;
  Pca9539rGpioAddress address = { .i2c_address = PCA9539_I2C_ADDRESS };
  for (Pca9539rPinAddress pin = PCA9539R_PIN_IO0_0; pin < NUM_PCA9539R_GPIO_PINS; pin++) {
    address.pin = pin;
    status_ok_or_return(pca9539r_gpio_get_state(&address, &in_state));
    if (state == in_state) {
      LOG_DEBUG("State for pin %d_%d set and read correctly\n", address.pin / 8, address.pin % 8);
    } else {
      LOG_DEBUG("State for pin %d_%d incorrect\n", address.pin / 8, address.pin % 8);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }
  return STATUS_CODE_OK;
}

// used to test proper gpio toggling. A multi-meter will be needed
static void prv_soft_timer_callback_output(SoftTimerId timer_id, void *context) {
  Pca9539rGpioState *state = (Pca9539rGpioState *)context;
  Pca9539rGpioState get_state;
  Pca9539rGpioAddress address = {
    .i2c_address = PCA9539_I2C_ADDRESS,
  };

  for (Pca9539rPinAddress pin = PCA9539R_PIN_IO0_0; pin < NUM_PCA9539R_GPIO_PINS; pin++) {
    address.pin = pin;
    pca9539r_gpio_toggle_state(&address);
    pca9539r_gpio_get_state(&address, &get_state);
    if (get_state == *state) {
      LOG_DEBUG("State for pin %d_%d set and read correctly\n", address.pin / 8, address.pin % 8);
    } else {
      LOG_DEBUG("State for pin %d_%d incorrect\n", address.pin / 8, address.pin % 8);
    }
  }
  *state =
      (*state == PCA9539R_GPIO_STATE_HIGH ? PCA9539R_GPIO_STATE_LOW : PCA9539R_GPIO_STATE_HIGH);
  LOG_DEBUG("GPIO state = %d\n", *state);
  soft_timer_start_millis(WAIT_TIME_MILLIS, prv_soft_timer_callback_output, state, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();
  setup_test();

  LOG_DEBUG("Testing GPIO initialization...\n");
  LOG_DEBUG("Initializing all pins out...\n");
  pca9539r_init_all_pins(PCA9539R_GPIO_DIR_OUT);
  pca9539r_check_all_pin_states(PCA9539R_GPIO_STATE_HIGH);
  LOG_DEBUG("GPIO initialization complete. Now beginning toggling of GPIO states\n");
  Pca9539rGpioState state;

  // Toggles gpio, compares expected values against registers, first read should be ignored
  soft_timer_start_millis(100, prv_soft_timer_callback_output, &state, NULL);

  while (true) {
    wait();
  }

  return 0;
}
