#include "bts_7200_current_sense.h"
#include <stddef.h>

#define STM32_GPIO_STATE_SELECT_OUT_0 GPIO_STATE_LOW
#define STM32_GPIO_STATE_SELECT_OUT_1 GPIO_STATE_HIGH
#define PCA9539R_GPIO_STATE_SELECT_OUT_0 PCA9539R_GPIO_STATE_LOW
#define PCA9539R_GPIO_STATE_SELECT_OUT_1 PCA9539R_GPIO_STATE_HIGH

static void prv_measure_current(SoftTimerId timer_id, void *context) {
  Bts7200Storage *storage = context;
  bts_7200_get_measurement(storage, &storage->reading_out_0, &storage->reading_out_1);

  if (storage->callback) {
    storage->callback(storage->reading_out_0, storage->reading_out_1, storage->callback_context);
  }

  soft_timer_start(storage->interval_us, &prv_measure_current, storage, &storage->timer_id);
}

static StatusCode prv_init_common(Bts7200Storage *storage) {
  // make sure that if the user calls stop() it doesn't cancel some other timer
  storage->timer_id = SOFT_TIMER_INVALID_TIMER;

  // initialize the sense pin
  GpioSettings sense_settings = {
    .direction = GPIO_DIR_IN,
    .alt_function = GPIO_ALTFN_ANALOG,
  };
  status_ok_or_return(gpio_init_pin(storage->sense_pin, &sense_settings));

  // initialize the sense pin as ADC
  AdcChannel sense_channel = NUM_ADC_CHANNELS;
  adc_get_channel(*storage->sense_pin, &sense_channel);
  adc_set_channel(sense_channel, true);
  return STATUS_CODE_OK;
}

// Handle fault by disabling output and waiting for
// BTS7200_FAULT_RESTART_DELAY_US
static StatusCode prv_bts_7200_handle_fault(Bts7200Storage *storage, bool fault0, bool fault1) {
  if (fault0) {
    // Storage for previous state of IN pin
    bool prev_gpio_state = 0;
    prev_gpio_state = bts_7200_get_output_0_enabled(storage);
    // Pull IN0 low + wait for BTS7200_FAULT_RESTART_DELAY_US
    status_ok_or_return(bts_7200_disable_output_0(storage));
    delay_us(BTS7200_FAULT_RESTART_DELAY_US);

    // If output was enabled before fault, re-enable
    if (prev_gpio_state) {
      status_ok_or_return(bts_7200_enable_output_0(storage));
    }
  }

  if (fault1) {
    // Storage for previous state of IN pin
    bool prev_gpio_state = 0;
    prev_gpio_state = bts_7200_get_output_1_enabled(storage);

    // Pull IN0 low + wait for BTS7200_FAULT_RESTART_DELAY_US
    status_ok_or_return(bts_7200_disable_output_1(storage));
    delay_us(BTS7200_FAULT_RESTART_DELAY_US);

    // If output was enabled before fault, re-enable
    if (prev_gpio_state) {
      status_ok_or_return(bts_7200_enable_output_1(storage));
    }
  }
  return STATUS_CODE_OK;
}

StatusCode bts_7200_init_stm32(Bts7200Storage *storage, Bts7200Stm32Settings *settings) {
  storage->select_pin_stm32 = settings->select_pin;
  storage->select_pin_pca9539r = NULL;
  storage->select_pin_type = BTS7200_SELECT_PIN_STM32;
  storage->sense_pin = settings->sense_pin;
  storage->input_0_pin_stm32 = settings->input_0_pin;
  storage->input_1_pin_stm32 = settings->input_1_pin;

  storage->interval_us = settings->interval_us;
  storage->callback = settings->callback;
  storage->callback_context = settings->callback_context;
  storage->fault_callback = settings->fault_callback;
  storage->fault_callback_context = settings->fault_callback_context;

  // initialize the select and input pins
  GpioSettings select_input_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  status_ok_or_return(gpio_init_pin(storage->select_pin_stm32, &select_input_settings));
  status_ok_or_return(gpio_init_pin(storage->input_0_pin_stm32, &select_input_settings));
  status_ok_or_return(gpio_init_pin(storage->input_1_pin_stm32, &select_input_settings));

  return prv_init_common(storage);
}

StatusCode bts_7200_init_pca9539r(Bts7200Storage *storage, Bts7200Pca9539rSettings *settings) {
  storage->select_pin_pca9539r = settings->select_pin;
  storage->select_pin_stm32 = NULL;
  storage->select_pin_type = BTS7200_SELECT_PIN_PCA9539R;
  storage->sense_pin = settings->sense_pin;
  storage->input_0_pin_pca9539r = settings->input_0_pin;
  storage->input_1_pin_pca9539r = settings->input_1_pin;

  storage->interval_us = settings->interval_us;
  storage->callback = settings->callback;
  storage->callback_context = settings->callback_context;
  storage->fault_callback = settings->fault_callback;
  storage->fault_callback_context = settings->fault_callback_context;

  // initialize PCA9539R on the relevant port
  pca9539r_gpio_init(settings->i2c_port, storage->select_pin_pca9539r->i2c_address);

  // initialize the select and input pins
  Pca9539rGpioSettings select_input_settings = {
    .direction = PCA9539R_GPIO_DIR_OUT,
    .state = PCA9539R_GPIO_STATE_LOW,
  };

  status_ok_or_return(pca9539r_gpio_init_pin(storage->select_pin_pca9539r, &select_input_settings));
  status_ok_or_return(pca9539r_gpio_init_pin(storage->input_0_pin_pca9539r, &select_input_settings));
  status_ok_or_return(pca9539r_gpio_init_pin(storage->input_1_pin_pca9539r, &select_input_settings));

  return prv_init_common(storage);
}

StatusCode bts_7200_enable_output_0(Bts7200Storage *storage) {
  if (storage->select_pin_type == BTS7200_SELECT_PIN_STM32) {
    return gpio_set_state(storage->input_0_pin_stm32, GPIO_STATE_HIGH);
  }
  return pca9539r_gpio_set_state(storage->input_0_pin_pca9539r, PCA9539R_GPIO_STATE_HIGH);
}

StatusCode bts_7200_enable_output_1(Bts7200Storage *storage) {
  if (storage->select_pin_type == BTS7200_SELECT_PIN_STM32) {
    return gpio_set_state(storage->input_1_pin_stm32, GPIO_STATE_HIGH);
  }
  return pca9539r_gpio_set_state(storage->input_1_pin_pca9539r, PCA9539R_GPIO_STATE_HIGH);
}

StatusCode bts_7200_disable_output_0(Bts7200Storage *storage) {
  if (storage->select_pin_type == BTS7200_SELECT_PIN_STM32) {
    return gpio_set_state(storage->input_0_pin_stm32, GPIO_STATE_LOW);
  }
  return pca9539r_gpio_set_state(storage->input_0_pin_pca9539r, PCA9539R_GPIO_STATE_LOW);
}

StatusCode bts_7200_disable_output_1(Bts7200Storage *storage) {
  if (storage->select_pin_type == BTS7200_SELECT_PIN_STM32) {
    return gpio_set_state(storage->input_0_pin_stm32, GPIO_STATE_LOW);
  }
  return pca9539r_gpio_set_state(storage->input_0_pin_pca9539r, PCA9539R_GPIO_STATE_LOW);
}

bool bts_7200_get_output_0_enabled(Bts7200Storage *storage) {
  if (storage->select_pin_type == BTS7200_SELECT_PIN_STM32) {
    GpioState pin_state;
    gpio_get_state(storage->input_0_pin_stm32, &pin_state);
    return (pin_state == GPIO_STATE_HIGH);
  } else {
    Pca9539rGpioState pin_state;
    pca9539r_gpio_get_state(storage->input_0_pin_pca9539r, &pin_state);
    return (pin_state == PCA9539R_GPIO_STATE_HIGH);
  }
}

bool bts_7200_get_output_1_enabled(Bts7200Storage *storage) {
  if (storage->select_pin_type == BTS7200_SELECT_PIN_STM32) {
    GpioState pin_state;
    gpio_get_state(storage->input_1_pin_stm32, &pin_state);
    return (pin_state == GPIO_STATE_HIGH);
  } else {
    Pca9539rGpioState pin_state;
    pca9539r_gpio_get_state(storage->input_1_pin_pca9539r, &pin_state);
    return (pin_state == PCA9539R_GPIO_STATE_HIGH);
  }
}

StatusCode bts_7200_get_measurement(Bts7200Storage *storage, uint16_t *meas0, uint16_t *meas1) {
  AdcChannel sense_channel = NUM_ADC_CHANNELS;
  adc_get_channel(*storage->sense_pin, &sense_channel);

  if (storage->select_pin_type == BTS7200_SELECT_PIN_STM32) {
    gpio_set_state(storage->select_pin_stm32, STM32_GPIO_STATE_SELECT_OUT_0);
  } else {
    pca9539r_gpio_set_state(storage->select_pin_pca9539r, PCA9539R_GPIO_STATE_SELECT_OUT_0);
  }
  adc_read_raw(sense_channel, meas0);

  if (storage->select_pin_type == BTS7200_SELECT_PIN_STM32) {
    gpio_set_state(storage->select_pin_stm32, STM32_GPIO_STATE_SELECT_OUT_1);
  } else {
    pca9539r_gpio_set_state(storage->select_pin_pca9539r, PCA9539R_GPIO_STATE_SELECT_OUT_1);
  }
  adc_read_raw(sense_channel, meas1);

  // Check for faults, call callback and handle fault if faulted
  bool fault0 = BTS7200_IS_MEASUREMENT_FAULT(*meas0);
  bool fault1 = BTS7200_IS_MEASUREMENT_FAULT(*meas1);

  if (fault0 || fault1) {
    storage->fault_callback(fault0, fault1, storage->fault_callback_context);
    prv_bts_7200_handle_fault(storage, fault0, fault1);
  }

  return STATUS_CODE_OK;
}


StatusCode bts_7200_start(Bts7200Storage *storage) {
  // |prv_measure_current| will set up a soft timer to call itself repeatedly
  // by calling this now there's no period with invalid measurements
  prv_measure_current(SOFT_TIMER_INVALID_TIMER, storage);
  return STATUS_CODE_OK;
}

bool bts_7200_stop(Bts7200Storage *storage) {
  bool result = soft_timer_cancel(storage->timer_id);
  // make sure calling stop twice doesn't cancel an unrelated timer
  storage->timer_id = SOFT_TIMER_INVALID_TIMER;
  return result;
}
