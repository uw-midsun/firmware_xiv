#include "bts7xxx_common.h"

// To use when initializing pins to be used through a PCA9539R.
const Pca9539rGpioSettings ENABLE_PIN_SETTINGS_PCA9539R = {
  .direction = PCA9539R_GPIO_DIR_OUT,
  .state = PCA9539R_GPIO_STATE_LOW,
};

// Same, but for STM32.
const GpioSettings ENABLE_PIN_SETTINGS_STM32 = {
  .direction = GPIO_DIR_OUT,
  .state = GPIO_STATE_LOW,
  .resistor = GPIO_RES_NONE,
  .alt_function = GPIO_ALTFN_NONE,
};

StatusCode bts7xxx_init_pin(Bts7xxxEnablePin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    return gpio_init_pin(pin->enable_pin_stm32, &ENABLE_PIN_SETTINGS_STM32);
  } else {
    return pca9539r_gpio_init_pin(pin->enable_pin_pca9539r, &ENABLE_PIN_SETTINGS_PCA9539R);
  }
}
StatusCode bts7xxx_enable_pin(Bts7xxxEnablePin *pin) {
  // Don't allow enabling pin on fault
  if (pin->fault_in_progress) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    return gpio_set_state(pin->enable_pin_stm32, GPIO_STATE_HIGH);
  } else {
    return pca9539r_gpio_set_state(pin->enable_pin_pca9539r, PCA9539R_GPIO_STATE_HIGH);
  }
}

StatusCode bts7xxx_disable_pin(Bts7xxxEnablePin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    return gpio_set_state(pin->enable_pin_stm32, GPIO_STATE_LOW);
  } else {
    return pca9539r_gpio_set_state(pin->enable_pin_pca9539r, PCA9539R_GPIO_STATE_LOW);
  }
}

StatusCode bts7xxx_get_pin_enabled(Bts7xxxEnablePin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    GpioState pin_state;
    gpio_get_state(pin->enable_pin_stm32, &pin_state);
    return (pin_state == GPIO_STATE_HIGH);
  } else {
    Pca9539rGpioState pin_state;
    pca9539r_gpio_get_state(pin->enable_pin_pca9539r, &pin_state);
    return (pin_state == PCA9539R_GPIO_STATE_HIGH);
  }
}
