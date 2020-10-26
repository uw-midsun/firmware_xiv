#include "bts7xxx_common.h"

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
