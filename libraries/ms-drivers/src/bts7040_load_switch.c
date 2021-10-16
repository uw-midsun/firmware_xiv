#include "bts7040_load_switch.h"

static void prv_measure_current(SoftTimerId timer_id, void *context) {
  Bts7040Storage *storage = context;
  bts7040_get_measurement(storage, &storage->reading_out);

  if (storage->callback != NULL) {
    storage->callback(storage->reading_out, storage->callback_context);
  }

  soft_timer_start(storage->interval_us, &prv_measure_current, storage,
                   &storage->measurement_timer_id);
}

// Init common elements of the load switch to STM32, PCA9539R implementations.
static StatusCode prv_init_common(Bts7040Storage *storage) {
  // make sure that if the user calls stop() it doesn't cancel some other timer
  storage->measurement_timer_id = SOFT_TIMER_INVALID_TIMER;

  // Timer for fault handling
  storage->enable_pin.fault_timer_id = SOFT_TIMER_INVALID_TIMER;

  storage->enable_pin.fault_in_progress = false;

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

StatusCode bts7040_init_stm32(Bts7040Storage *storage, Bts7040Stm32Settings *settings) {
  storage->sense_pin = settings->sense_pin;

  storage->enable_pin.enable_pin_stm32 = settings->enable_pin;
  storage->enable_pin.enable_pin_pca9539r = NULL;

  storage->enable_pin.pin_type = BTS7XXX_PIN_STM32;

  storage->interval_us = settings->interval_us;
  storage->callback = settings->callback;
  storage->callback_context = settings->callback_context;
  storage->fault_callback = settings->fault_callback;
  storage->fault_callback_context = settings->fault_callback_context;

  storage->resistor = settings->resistor;
  storage->bias = settings->bias;

  storage->min_fault_voltage_mv = settings->min_fault_voltage_mv;

  storage->use_bts7004_scaling = settings->use_bts7004_scaling;

  // Initialize the enable pin
  status_ok_or_return(bts7xxx_init_pin(&storage->enable_pin));

  return prv_init_common(storage);
}

StatusCode bts7040_init_pca9539r(Bts7040Storage *storage, Bts7040Pca9539rSettings *settings) {
  storage->sense_pin = settings->sense_pin;
  storage->enable_pin.enable_pin_pca9539r = settings->enable_pin;
  storage->enable_pin.enable_pin_stm32 = NULL;

  storage->enable_pin.pin_type = BTS7XXX_PIN_PCA9539R;

  storage->interval_us = settings->interval_us;
  storage->callback = settings->callback;
  storage->callback_context = settings->callback_context;
  storage->fault_callback = settings->fault_callback;
  storage->fault_callback_context = settings->fault_callback_context;

  storage->resistor = settings->resistor;
  storage->bias = settings->bias;

  storage->min_fault_voltage_mv = settings->min_fault_voltage_mv;

  storage->use_bts7004_scaling = settings->use_bts7004_scaling;

  // initialize PCA9539R on the relevant port
  pca9539r_gpio_init(settings->i2c_port, storage->enable_pin.enable_pin_pca9539r->i2c_address);

  // initialize the enable pin
  status_ok_or_return(bts7xxx_init_pin(&storage->enable_pin));

  return prv_init_common(storage);
}

StatusCode bts7040_enable_output(Bts7040Storage *storage) {
  return bts7xxx_enable_pin(&storage->enable_pin);
}

StatusCode bts7040_disable_output(Bts7040Storage *storage) {
  return bts7xxx_disable_pin(&storage->enable_pin);
}

bool bts7040_get_output_enabled(Bts7040Storage *storage) {
  return bts7xxx_get_pin_enabled(&storage->enable_pin);
}

// Convert voltage measurements to current
static void prv_convert_voltage_to_current(Bts7040Storage *storage, uint16_t *meas) {
  if (*meas <= BTS7040_MAX_LEAKAGE_VOLTAGE_MV) {
    *meas = 0;
  } else {
    // using 32 bits to avoid overflow, and signed ints to get around C's janky type system
    uint32_t meas32 = (uint32_t)*meas;
    meas32 *=
        (storage->use_bts7004_scaling ? BTS7004_IS_SCALING_NOMINAL : BTS7040_IS_SCALING_NOMINAL);
    meas32 /= storage->resistor;
    int32_t unbiased_meas32 = (int32_t)meas32 - storage->bias;
    *meas = (uint16_t)MAX(unbiased_meas32, 0);
  }
}

StatusCode bts7040_get_measurement(Bts7040Storage *storage, uint16_t *meas) {
  status_ok_or_return(adc_read_converted_pin(*storage->sense_pin, meas));

  // Set equal to 0 if below/equal to leakage current.  Otherwise, convert to true load current.
  // Check for faults, call callback and handle fault if voltage is within fault range
  if (*meas >= storage->min_fault_voltage_mv) {
    // We don't really need to convert the voltage here, but this is consistent with the BTS7200
    // driver's behaviour.
    prv_convert_voltage_to_current(storage, meas);

    // Only call fault cb if it's not NULL
    if (storage->fault_callback != NULL) {
      storage->fault_callback(storage->fault_callback_context);
    }

    // Handle fault, return either the error from the fault handling
    // or STATUS_CODE_INTERNAL_ERROR if the fault pin process works OK
    status_ok_or_return(bts7xxx_handle_fault_pin(&storage->enable_pin));
    return STATUS_CODE_INTERNAL_ERROR;
  }

  prv_convert_voltage_to_current(storage, meas);

  return STATUS_CODE_OK;
}

StatusCode bts7040_start(Bts7040Storage *storage) {
  // |prv_measure_current| will set up a soft timer to call itself repeatedly
  // by calling this now there's no period with invalid measurements
  prv_measure_current(SOFT_TIMER_INVALID_TIMER, storage);
  return STATUS_CODE_OK;
}

void bts7040_stop(Bts7040Storage *storage) {
  soft_timer_cancel(storage->measurement_timer_id);
  soft_timer_cancel(storage->enable_pin.fault_timer_id);

  // make sure calling stop twice doesn't cancel an unrelated timer
  storage->measurement_timer_id = SOFT_TIMER_INVALID_TIMER;
  storage->enable_pin.fault_timer_id = SOFT_TIMER_INVALID_TIMER;

  // Fault handling no longer in progress
  storage->enable_pin.fault_in_progress = false;
}
