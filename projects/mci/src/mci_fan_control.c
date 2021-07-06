#include "mci_fan_control.h"

#include "gpio.h"

// Stores fault types
static uint8_t fault_bitset;

StatusCode mci_fan_control_init(MciFanControlSettings *settings) {
  // Currently, just set fan to 100%
  GpioSettings pin_settings = {
    .direction = GPIO_DIR_OUT,
    .alt_function = GPIO_ALTFN_NONE,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
  };
  GpioAddress en_addr = MCI_FAN_EN_ADDR;

  status_ok_or_return(gpio_init_pin(&en_addr, &pin_settings));
  return gpio_set_state(&en_addr, GPIO_STATE_HIGH);
  // TODO(SOFT-275): set PWM to 100% if needed
  // TODO(SOFT-275): interrupts on thermistor overtemps if needed
}
