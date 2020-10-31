#include "bts7200_load_switch.h"

static void prv_measure_current(SoftTimerId timer_id, void *context) {
  Bts7200Storage *storage = context;
  bts7200_get_measurement(storage, &storage->reading_out_0, &storage->reading_out_1);

  if (storage->callback != NULL) {
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

// Handle + clear faults. After fault is cleared, input pin is returned to its initial state.
// Faults are cleared following the retry strategy on pg. 34 of the BTS7200 datasheet,
// assuming a worst-case t(DELAY(CR)) of BTS7200_FAULT_RESTART_DELAY_MS (pg. 38)
static StatusCode prv_bts7200_handle_fault(Bts7200Storage *storage, bool fault0, bool fault1) {
  StatusCode status0 = STATUS_CODE_OK;
  StatusCode status1 = STATUS_CODE_OK;

  if (fault0) {
    status0 = bts7xxx_handle_fault_pin(&storage->enable_pin_0);
  }

  if (fault1) {
    status1 = bts7xxx_handle_fault_pin(&storage->enable_pin_1);
  }

  // Only return on non-OK status codes after trying to clear faults on both pins
  status_ok_or_return(status0);
  status_ok_or_return(status1);
  return STATUS_CODE_OK;
}

StatusCode bts7200_init_stm32(Bts7200Storage *storage, Bts7200Stm32Settings *settings) {
  storage->select_pin.select_pin_stm32 = settings->select_pin;
  storage->select_pin.select_pin_pca9539r = NULL;
  storage->select_pin.pin_type = BTS7XXX_PIN_STM32;

  storage->sense_pin = settings->sense_pin;

  storage->enable_pin_0.enable_pin_stm32 = settings->enable_0_pin;
  storage->enable_pin_0.enable_pin_pca9539r = NULL;
  storage->enable_pin_1.enable_pin_stm32 = settings->enable_1_pin;
  storage->enable_pin_1.enable_pin_pca9539r = NULL;

  storage->enable_pin_0.pin_type = BTS7XXX_PIN_STM32;
  storage->enable_pin_1.pin_type = BTS7XXX_PIN_STM32;

  storage->interval_us = settings->interval_us;
  storage->callback = settings->callback;
  storage->callback_context = settings->callback_context;
  storage->fault_callback = settings->fault_callback;
  storage->fault_callback_context = settings->fault_callback_context;

  storage->resistor = settings->resistor;
  storage->bias = settings->bias;

  storage->min_fault_voltage_mv = settings->min_fault_voltage_mv;
  storage->max_fault_voltage_mv = settings->max_fault_voltage_mv;

  // initialize the select and input pins
  GpioSettings select_enable_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  status_ok_or_return(gpio_init_pin(storage->select_pin.select_pin_stm32, &select_enable_settings));

  status_ok_or_return(bts7xxx_init_pin(&storage->enable_pin_0));
  status_ok_or_return(bts7xxx_init_pin(&storage->enable_pin_1));

  return prv_init_common(storage);
}

StatusCode bts7200_init_pca9539r(Bts7200Storage *storage, Bts7200Pca9539rSettings *settings) {
  storage->select_pin.select_pin_pca9539r = settings->select_pin;
  storage->select_pin.select_pin_stm32 = NULL;
  storage->select_pin.pin_type = BTS7XXX_PIN_PCA9539R;

  storage->sense_pin = settings->sense_pin;

  storage->enable_pin_0.enable_pin_pca9539r = settings->enable_0_pin;
  storage->enable_pin_0.enable_pin_stm32 = NULL;
  storage->enable_pin_1.enable_pin_pca9539r = settings->enable_1_pin;
  storage->enable_pin_1.enable_pin_stm32 = NULL;

  storage->enable_pin_0.pin_type = BTS7XXX_PIN_PCA9539R;
  storage->enable_pin_1.pin_type = BTS7XXX_PIN_PCA9539R;

  storage->interval_us = settings->interval_us;
  storage->callback = settings->callback;
  storage->callback_context = settings->callback_context;
  storage->fault_callback = settings->fault_callback;
  storage->fault_callback_context = settings->fault_callback_context;

  storage->resistor = settings->resistor;
  storage->bias = settings->bias;

  storage->min_fault_voltage_mv = settings->min_fault_voltage_mv;
  storage->max_fault_voltage_mv = settings->max_fault_voltage_mv;
  // initialize PCA9539R on the relevant ports
  status_ok_or_return(
      pca9539r_gpio_init(settings->i2c_port, storage->select_pin.select_pin_pca9539r->i2c_address));
  status_ok_or_return(pca9539r_gpio_init(settings->i2c_port,
                                         storage->enable_pin_0.enable_pin_pca9539r->i2c_address));
  status_ok_or_return(pca9539r_gpio_init(settings->i2c_port,
                                         storage->enable_pin_1.enable_pin_pca9539r->i2c_address));

  // initialize the select pin
  Pca9539rGpioSettings select_settings = {
    .direction = PCA9539R_GPIO_DIR_OUT,
    .state = PCA9539R_GPIO_STATE_LOW,
  };

  status_ok_or_return(
      pca9539r_gpio_init_pin(storage->select_pin.select_pin_pca9539r, &select_settings));

  status_ok_or_return(bts7xxx_init_pin(&storage->enable_pin_0));
  status_ok_or_return(bts7xxx_init_pin(&storage->enable_pin_1));

  return prv_init_common(storage);
}

StatusCode bts7200_enable_output_0(Bts7200Storage *storage) {
  return bts7xxx_enable_pin(&storage->enable_pin_0);
}

StatusCode bts7200_enable_output_1(Bts7200Storage *storage) {
  return bts7xxx_enable_pin(&storage->enable_pin_1);
}

StatusCode bts7200_disable_output_0(Bts7200Storage *storage) {
  return bts7xxx_disable_pin(&storage->enable_pin_0);
}

StatusCode bts7200_disable_output_1(Bts7200Storage *storage) {
  return bts7xxx_disable_pin(&storage->enable_pin_1);
}

bool bts7200_get_output_0_enabled(Bts7200Storage *storage) {
  return bts7xxx_get_pin_enabled(&storage->enable_pin_0);
}

bool bts7200_get_output_1_enabled(Bts7200Storage *storage) {
  return bts7xxx_get_pin_enabled(&storage->enable_pin_1);
}

// Convert voltage measurements to current
static void prv_convert_voltage_to_current(Bts7200Storage *storage, uint16_t *meas) {
  if (*meas <= BTS7200_MAX_LEAKAGE_VOLTAGE_MV) {
    *meas = 0;
  } else {
    // using uint32_t to avoid overflow when multiplying by nominal scaling factor
    uint32_t meas32 = (uint32_t)*meas;
    meas32 *= BTS7200_IS_SCALING_NOMINAL;
    meas32 /= storage->resistor;
    meas32 -= storage->bias;
    *meas = (uint16_t)meas32;
  }
}

StatusCode bts7200_get_measurement(Bts7200Storage *storage, uint16_t *meas0, uint16_t *meas1) {
  AdcChannel sense_channel = NUM_ADC_CHANNELS;
  status_ok_or_return(adc_get_channel(*storage->sense_pin, &sense_channel));

  if (storage->select_pin.pin_type == BTS7XXX_PIN_STM32) {
    gpio_set_state(storage->select_pin.select_pin_stm32, STM32_GPIO_STATE_SELECT_OUT_0);
  } else {
    pca9539r_gpio_set_state(storage->select_pin.select_pin_pca9539r,
                            PCA9539R_GPIO_STATE_SELECT_OUT_0);
  }

  status_ok_or_return(adc_read_converted(sense_channel, meas0));

  if (storage->select_pin.pin_type == BTS7XXX_PIN_STM32) {
    gpio_set_state(storage->select_pin.select_pin_stm32, STM32_GPIO_STATE_SELECT_OUT_1);
  } else {
    pca9539r_gpio_set_state(storage->select_pin.select_pin_pca9539r,
                            PCA9539R_GPIO_STATE_SELECT_OUT_1);
  }

  status_ok_or_return(adc_read_converted(sense_channel, meas1));
  // Set equal to 0 if below/equal to leakage current.  Otherwise, convert to true load current.
  // Check for faults, call callback and handle fault if voltage is within fault range
  bool fault0 =
      (storage->min_fault_voltage_mv <= *meas0) && (*meas0 <= storage->max_fault_voltage_mv);
  bool fault1 =
      (storage->min_fault_voltage_mv <= *meas1) && (*meas1 <= storage->max_fault_voltage_mv);

  prv_convert_voltage_to_current(storage, meas0);
  prv_convert_voltage_to_current(storage, meas1);

  if (fault0 || fault1) {
    // Only call fault cb if it's not NULL
    if (storage->fault_callback != NULL) {
      storage->fault_callback(fault0, fault1, storage->fault_callback_context);
    }
    // Handle fault
    prv_bts7200_handle_fault(storage, fault0, fault1);

    // Return internal error on fault
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode bts7200_start(Bts7200Storage *storage) {
  // |prv_measure_current| will set up a soft timer to call itself repeatedly
  // by calling this now there's no period with invalid measurements
  prv_measure_current(SOFT_TIMER_INVALID_TIMER, storage);
  return STATUS_CODE_OK;
}

void bts7200_stop(Bts7200Storage *storage) {
  soft_timer_cancel(storage->measurement_timer_id);
  soft_timer_cancel(storage->enable_pin_0.fault_timer_id);
  soft_timer_cancel(storage->enable_pin_1.fault_timer_id);
  // make sure calling stop twice doesn't cancel an unrelated timer
  storage->measurement_timer_id = SOFT_TIMER_INVALID_TIMER;
  storage->enable_pin_0.fault_timer_id = SOFT_TIMER_INVALID_TIMER;
  storage->enable_pin_1.fault_timer_id = SOFT_TIMER_INVALID_TIMER;

  // Fault handling no longer in progress
  storage->enable_pin_0.fault_in_progress = false;
  storage->enable_pin_1.fault_in_progress = false;
}
