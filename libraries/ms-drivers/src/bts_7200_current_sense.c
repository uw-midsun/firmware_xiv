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

  soft_timer_start(storage->interval_us, &prv_measure_current, storage,
                   &storage->measurement_timer_id);
}

static StatusCode prv_init_common(Bts7200Storage *storage) {
  // make sure that if the user calls stop() it doesn't cancel some other timer
  storage->measurement_timer_id = SOFT_TIMER_INVALID_TIMER;

  // Timers for fault handling
  storage->enable_pin_0.fault_timer_id = SOFT_TIMER_INVALID_TIMER;
  storage->enable_pin_1.fault_timer_id = SOFT_TIMER_INVALID_TIMER;

  storage->enable_pin_0.fault_in_progress = false;
  storage->enable_pin_1.fault_in_progress = false;

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

// Broad function to enable the pin passed in.
static StatusCode prv_bts_7200_enable_pin(Bts7200EnablePin *pin) {
  if (pin->pin_type == BTS7200_PIN_STM32) {
    return gpio_set_state(pin->enable_pin_stm32, GPIO_STATE_HIGH);
  } else {
    return pca9539r_gpio_set_state(pin->enable_pin_pca9539r, PCA9539R_GPIO_STATE_HIGH);
  }
}

// Broad function to disable the pin passed in.
static StatusCode prv_bts_7200_disable_pin(Bts7200EnablePin *pin) {
  if (pin->pin_type == BTS7200_PIN_STM32) {
    return gpio_set_state(pin->enable_pin_stm32, GPIO_STATE_LOW);
  } else {
    return pca9539r_gpio_set_state(pin->enable_pin_pca9539r, PCA9539R_GPIO_STATE_LOW);
  }
}

// Broad function to get whether the pin passed in is enabled.
static StatusCode prv_bts_7200_get_pin_enabled(Bts7200EnablePin *pin) {
  if (pin->pin_type == BTS7200_PIN_STM32) {
    GpioState pin_state;
    gpio_get_state(pin->enable_pin_stm32, &pin_state);
    return (pin_state == GPIO_STATE_HIGH);
  } else {
    Pca9539rGpioState pin_state;
    pca9539r_gpio_get_state(pin->enable_pin_pca9539r, &pin_state);
    return (pin_state == PCA9539R_GPIO_STATE_HIGH);
  }
}

// Broad pin soft timer cb without re-enabling the pin
static void prv_bts_7200_fault_handler_cb(SoftTimerId timer_id, void *context) {
  Bts7200EnablePin *pin = context;
  pin->fault_in_progress = false;
  pin->fault_timer_id = SOFT_TIMER_INVALID_TIMER;
}

// Broad pin re-enable soft timer cb
static void prv_bts_7200_fault_handler_enable_cb(SoftTimerId timer_id, void *context) {
  Bts7200EnablePin *pin = context;
  prv_bts_7200_fault_handler_cb(timer_id, context);
  prv_bts_7200_enable_pin(pin);
}

// Helper function to clear fault on a given pin
static StatusCode prv_bts_7200_handle_fault_pin(Bts7200EnablePin *pin) {
  if (!pin->fault_in_progress) {
    pin->fault_in_progress = true;
    if (prv_bts_7200_get_pin_enabled(pin)) {
      status_ok_or_return(prv_bts_7200_disable_pin(pin));
      soft_timer_start(BTS7200_FAULT_RESTART_DELAY_US, prv_bts_7200_fault_handler_enable_cb, pin,
                       &pin->fault_timer_id);
    } else {
      soft_timer_start(BTS7200_FAULT_RESTART_DELAY_US, prv_bts_7200_fault_handler_cb, pin,
                       &pin->fault_timer_id);
    }
  }
  return STATUS_CODE_OK;
}
// Handle + clear faults. After fault is cleared, input pin is returned to its initial state.
// Faults are cleared following the retry strategy on pg. 34 of the BTS7200 datasheet,
// assuming a worst-case t(DELAY(CR)) of BTS7200_FAULT_RESTART_DELAY_MS (pg. 38)
static StatusCode prv_bts_7200_handle_fault(Bts7200Storage *storage, bool fault0, bool fault1) {
  StatusCode status0 = STATUS_CODE_OK;
  StatusCode status1 = STATUS_CODE_OK;

  if (fault0) {
    status0 = prv_bts_7200_handle_fault_pin(&storage->enable_pin_0);
  }

  if (fault1) {
    status1 = prv_bts_7200_handle_fault_pin(&storage->enable_pin_1);
  }

  // Only return on non-OK statuscodes after trying to clear faults on both pins
  status_ok_or_return(status0);
  status_ok_or_return(status1);
  return STATUS_CODE_OK;
}

StatusCode bts_7200_init_stm32(Bts7200Storage *storage, Bts7200Stm32Settings *settings) {
  storage->select_pin.select_pin_stm32 = settings->select_pin;
  storage->select_pin.select_pin_pca9539r = NULL;
  storage->select_pin.pin_type = BTS7200_PIN_STM32;

  storage->sense_pin = settings->sense_pin;

  storage->enable_pin_0.enable_pin_stm32 = settings->enable_0_pin;
  storage->enable_pin_0.enable_pin_pca9539r = NULL;
  storage->enable_pin_1.enable_pin_stm32 = settings->enable_1_pin;
  storage->enable_pin_1.enable_pin_pca9539r = NULL;

  storage->enable_pin_0.pin_type = BTS7200_PIN_STM32;
  storage->enable_pin_1.pin_type = BTS7200_PIN_STM32;

  storage->interval_us = settings->interval_us;
  storage->callback = settings->callback;
  storage->callback_context = settings->callback_context;
  storage->fault_callback = settings->fault_callback;
  storage->fault_callback_context = settings->fault_callback_context;

  storage->resistor = settings->resistor;

  // initialize the select and input pins
  GpioSettings select_enable_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  status_ok_or_return(gpio_init_pin(storage->select_pin.select_pin_stm32, &select_enable_settings));
  status_ok_or_return(
      gpio_init_pin(storage->enable_pin_0.enable_pin_stm32, &select_enable_settings));
  status_ok_or_return(
      gpio_init_pin(storage->enable_pin_1.enable_pin_stm32, &select_enable_settings));

  return prv_init_common(storage);
}

StatusCode bts_7200_init_pca9539r(Bts7200Storage *storage, Bts7200Pca9539rSettings *settings) {
  storage->select_pin.select_pin_pca9539r = settings->select_pin;
  storage->select_pin.select_pin_stm32 = NULL;
  storage->select_pin.pin_type = BTS7200_PIN_PCA9539R;

  storage->sense_pin = settings->sense_pin;

  storage->enable_pin_0.enable_pin_pca9539r = settings->enable_0_pin;
  storage->enable_pin_0.enable_pin_stm32 = NULL;
  storage->enable_pin_1.enable_pin_pca9539r = settings->enable_1_pin;
  storage->enable_pin_1.enable_pin_stm32 = NULL;

  storage->enable_pin_0.pin_type = BTS7200_PIN_PCA9539R;
  storage->enable_pin_1.pin_type = BTS7200_PIN_PCA9539R;

  storage->interval_us = settings->interval_us;
  storage->callback = settings->callback;
  storage->callback_context = settings->callback_context;
  storage->fault_callback = settings->fault_callback;
  storage->fault_callback_context = settings->fault_callback_context;

  storage->resistor = settings->resistor;

  // initialize PCA9539R on the relevant port
  pca9539r_gpio_init(settings->i2c_port, storage->select_pin.select_pin_pca9539r->i2c_address);

  // initialize the select and input pins
  Pca9539rGpioSettings select_enable_settings = {
    .direction = PCA9539R_GPIO_DIR_OUT,
    .state = PCA9539R_GPIO_STATE_LOW,
  };

  status_ok_or_return(
      pca9539r_gpio_init_pin(storage->select_pin.select_pin_pca9539r, &select_enable_settings));
  status_ok_or_return(
      pca9539r_gpio_init_pin(storage->enable_pin_0.enable_pin_pca9539r, &select_enable_settings));
  status_ok_or_return(
      pca9539r_gpio_init_pin(storage->enable_pin_1.enable_pin_pca9539r, &select_enable_settings));

  return prv_init_common(storage);
}

StatusCode bts_7200_enable_output_0(Bts7200Storage *storage) {
  if (storage->enable_pin_0.fault_in_progress) {
    return STATUS_CODE_INTERNAL_ERROR;
  } else {
    return prv_bts_7200_enable_pin(&storage->enable_pin_0);
  }
}

StatusCode bts_7200_enable_output_1(Bts7200Storage *storage) {
  if (storage->enable_pin_1.fault_in_progress) {
    return STATUS_CODE_INTERNAL_ERROR;
  } else {
    return prv_bts_7200_enable_pin(&storage->enable_pin_1);
  }
}

StatusCode bts_7200_disable_output_0(Bts7200Storage *storage) {
  return prv_bts_7200_disable_pin(&storage->enable_pin_0);
}

StatusCode bts_7200_disable_output_1(Bts7200Storage *storage) {
  return prv_bts_7200_disable_pin(&storage->enable_pin_1);
}

bool bts_7200_get_output_0_enabled(Bts7200Storage *storage) {
  return prv_bts_7200_get_pin_enabled(&storage->enable_pin_0);
}

bool bts_7200_get_output_1_enabled(Bts7200Storage *storage) {
  return prv_bts_7200_get_pin_enabled(&storage->enable_pin_1);
}

StatusCode bts_7200_get_measurement(Bts7200Storage *storage, uint16_t *meas0, uint16_t *meas1) {
  AdcChannel sense_channel = NUM_ADC_CHANNELS;
  status_ok_or_return(adc_get_channel(*storage->sense_pin, &sense_channel));

  if (storage->select_pin.pin_type == BTS7200_PIN_STM32) {
    gpio_set_state(storage->select_pin.select_pin_stm32, STM32_GPIO_STATE_SELECT_OUT_0);
  } else {
    pca9539r_gpio_set_state(storage->select_pin.select_pin_pca9539r,
                            PCA9539R_GPIO_STATE_SELECT_OUT_0);
  }
  status_ok_or_return(adc_read_converted(sense_channel, meas0));

  if (storage->select_pin.pin_type == BTS7200_PIN_STM32) {
    gpio_set_state(storage->select_pin.select_pin_stm32, STM32_GPIO_STATE_SELECT_OUT_1);
  } else {
    pca9539r_gpio_set_state(storage->select_pin.select_pin_pca9539r,
                            PCA9539R_GPIO_STATE_SELECT_OUT_1);
  }
  status_ok_or_return(adc_read_converted(sense_channel, meas1));
  // Set equal to 0 if below/equal to leakage current.  Otherwise, convert to true load current.
  // Check for faults, call callback and handle fault if faulted
  bool fault0 = BTS7200_IS_MEASUREMENT_FAULT_MV(*meas0);
  bool fault1 = BTS7200_IS_MEASUREMENT_FAULT_MV(*meas1);

  if (*meas0 <= BTS7200_MAX_LEAKAGE_VOLTAGE_MV) {
    *meas0 = 0;
  } else {
    (*meas0) *= BTS7200_IS_SCALING_NOMINAL;
    (*meas0) /= storage->resistor;
  }
  if (*meas1 <= BTS7200_MAX_LEAKAGE_VOLTAGE_MV) {
    *meas1 = 0;
  } else {
    (*meas1) *= BTS7200_IS_SCALING_NOMINAL;
    (*meas1) /= storage->resistor;
  }

  if (fault0 || fault1) {
    // Only call fault cb if it's not NULL
    if (storage->fault_callback != NULL) {
      storage->fault_callback(fault0, fault1, storage->fault_callback_context);
    }
    // Handle fault
    prv_bts_7200_handle_fault(storage, fault0, fault1);

    // Return internal error on fault
    return STATUS_CODE_INTERNAL_ERROR;
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
  bool result = soft_timer_cancel(storage->measurement_timer_id);
  soft_timer_cancel(storage->enable_pin_0.fault_timer_id);
  soft_timer_cancel(storage->enable_pin_1.fault_timer_id);
  // make sure calling stop twice doesn't cancel an unrelated timer
  storage->measurement_timer_id = SOFT_TIMER_INVALID_TIMER;
  storage->enable_pin_0.fault_timer_id = SOFT_TIMER_INVALID_TIMER;
  storage->enable_pin_1.fault_timer_id = SOFT_TIMER_INVALID_TIMER;

  // Fault handling no longer in progress
  storage->enable_pin_0.fault_in_progress = false;
  storage->enable_pin_1.fault_in_progress = false;

  return result;
}
