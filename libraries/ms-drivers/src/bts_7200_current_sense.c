#include "bts_7200_current_sense.h"
#include <stddef.h>
#include "log.h"

#define STM32_GPIO_STATE_SELECT_OUT_0 GPIO_STATE_LOW
#define STM32_GPIO_STATE_SELECT_OUT_1 GPIO_STATE_HIGH
#define PCA9539R_GPIO_STATE_SELECT_OUT_0 PCA9539R_GPIO_STATE_LOW
#define PCA9539R_GPIO_STATE_SELECT_OUT_1 PCA9539R_GPIO_STATE_HIGH

// Track whether faults are in progress 
// (just using this to see if it fixes issues with soft timers)
static bool s_enable_0_fault_in_progress = false;
static bool s_enable_1_fault_in_progress = false;

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

  // Timers for fault handling
  storage->fault_timer_0 = BTS7200_FAULT_INPUT_0_TIMER;
  storage->fault_timer_1 = BTS7200_FAULT_INPUT_1_TIMER;

  storage->fault_0_in_progress = false; 
  storage->fault_1_in_progress = false; 

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

// Callback for soft timer in prv_bts_7200_handle_fault to re-enable IN0
static void prv_bts_7200_fault_handler_enable_0_cb(SoftTimerId timer_id, void *context) {
  Bts7200Storage *storage = context;
  storage->fault_0_in_progress = false; 
  bts_7200_enable_output_0(storage);
}

// Callback for soft timer in prv_bts_7200_handle_fault to re-enable IN1
static void prv_bts_7200_fault_handler_enable_1_cb(SoftTimerId timer_id, void *context) {
  Bts7200Storage *storage = context;
  storage->fault_1_in_progress = false;
  bts_7200_enable_output_1(storage);
}

// Callback for soft timer in fault handler to leave IN0 disabled
static void prv_bts_7200_fault_handler_0_default_cb(SoftTimerId timer_id, void *context) {
  Bts7200Storage *storage = context;
  storage->fault_0_in_progress = false; 
}

// Callback for soft timer in fault handler to leave IN1 disabled
static void prv_bts_7200_fault_handler_1_default_cb(SoftTimerId timer_id, void *context) {
  Bts7200Storage *storage = context;
  storage->fault_1_in_progress = false;  
}

// Handle + clear faults. After fault is cleared, input pin is returned to its initial state.
static StatusCode prv_bts_7200_handle_fault(Bts7200Storage *storage, bool fault0, bool fault1) {
  LOG_DEBUG("FAULT CALLED\n");
  //LOG_DEBUG("Handling fault\n");
  if (fault0) {
    LOG_DEBUG("FAULT0\n");
    // Only start a new soft timer if the timer doesn't currently exist
    // (e.g. only handle fault if fault not currently being handled)
    if(!storage->fault_0_in_progress) {
      LOG_DEBUG("FAULT NOT IN PROGRESS\n");
      storage->fault_0_in_progress = true; 
      if(bts_7200_get_output_0_enabled(storage) == true) {
        LOG_DEBUG("OUTPUT 0 ENABLED, DISABLING\n");
        status_ok_or_return(bts_7200_disable_output_0(storage));
        soft_timer_start(BTS7200_FAULT_RESTART_DELAY_US, prv_bts_7200_fault_handler_enable_0_cb, storage, &storage->fault_timer_0);
      } else {
        soft_timer_start(BTS7200_FAULT_RESTART_DELAY_US, prv_bts_7200_fault_handler_0_default_cb, storage, &storage->fault_timer_0);
      }
    }
  }
  if (fault1) {
    LOG_DEBUG("FAULT1\n");
    // Only start a new soft timer if the timer doesn't currently exist
    // (e.g. only handle fault if fault not currently being handled)
    if(!storage->fault_1_in_progress) {
      LOG_DEBUG("FAULT1 not in PROGRESS\n");
      storage->fault_1_in_progress = true;
      if(bts_7200_get_output_1_enabled(storage)) {
        LOG_DEBUG("OUTPUT 1 ENABLED, DISABLING\n");
        status_ok_or_return(bts_7200_disable_output_1(storage));
        soft_timer_start(BTS7200_FAULT_RESTART_DELAY_US, prv_bts_7200_fault_handler_enable_1_cb, storage, &storage->fault_timer_1);
      } else {
        soft_timer_start(BTS7200_FAULT_RESTART_DELAY_US, prv_bts_7200_fault_handler_1_default_cb, storage, &storage->fault_timer_1);
      }
    }
  }
  return STATUS_CODE_OK;
}

bool bts_7200_get_fault_in_progress(Bts7200Storage *storage, bool *fault0, bool *fault1) {
  *fault0 = storage->fault_0_in_progress; 
  *fault1 = storage->fault_1_in_progress;
  return (fault0 || fault1); 
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
  LOG_DEBUG("ENABLE OUTPUT 0\n");
  if(!storage->fault_0_in_progress) {
    LOG_DEBUG("FAULT 0 NOT IN PROGRESS\n");
    bool test = storage->fault_0_in_progress;
    if (storage->select_pin_type == BTS7200_SELECT_PIN_STM32) {
      return gpio_set_state(storage->input_0_pin_stm32, GPIO_STATE_HIGH);
    }
    return pca9539r_gpio_set_state(storage->input_0_pin_pca9539r, PCA9539R_GPIO_STATE_HIGH);
  } else {
    LOG_DEBUG("returning internal error\n");
    return STATUS_CODE_INTERNAL_ERROR;
  }
}

StatusCode bts_7200_enable_output_1(Bts7200Storage *storage) {
  LOG_DEBUG("ENABLE OUTPUT 1\n");
  if(!storage->fault_1_in_progress) {
    LOG_DEBUG("FAULT 1 NOT IN PROGRESS\n");
    if (storage->select_pin_type == BTS7200_SELECT_PIN_STM32) {
      return gpio_set_state(storage->input_1_pin_stm32, GPIO_STATE_HIGH);
    }
    return pca9539r_gpio_set_state(storage->input_1_pin_pca9539r, PCA9539R_GPIO_STATE_HIGH);
  } else {
    LOG_DEBUG("returning internal error 1\n");
    return STATUS_CODE_INTERNAL_ERROR;
  }
}

StatusCode bts_7200_disable_output_0(Bts7200Storage *storage) {
  if (storage->select_pin_type == BTS7200_SELECT_PIN_STM32) {
    return gpio_set_state(storage->input_0_pin_stm32, GPIO_STATE_LOW);
  }
  return pca9539r_gpio_set_state(storage->input_0_pin_pca9539r, PCA9539R_GPIO_STATE_LOW);
}

StatusCode bts_7200_disable_output_1(Bts7200Storage *storage) {
  if (storage->select_pin_type == BTS7200_SELECT_PIN_STM32) {
    return gpio_set_state(storage->input_1_pin_stm32, GPIO_STATE_LOW);
  }
  return pca9539r_gpio_set_state(storage->input_1_pin_pca9539r, PCA9539R_GPIO_STATE_LOW);
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
  LOG_DEBUG("Getting measurement\n");
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

  //Check for faults, call callback and handle fault if faulted
  bool fault0 = BTS7200_IS_MEASUREMENT_FAULT(*meas0);
  bool fault1 = BTS7200_IS_MEASUREMENT_FAULT(*meas1);
  
  if (fault0 || fault1) {
    LOG_DEBUG("FAULT0: %d\n FAULT1: %d\n", fault0, fault1);
    ////LOG_DEBUG("calling fault cb\n");
    // Only call fault cb if it's not NULL
    if(storage->fault_callback) {
      storage->fault_callback(fault0, fault1, storage->fault_callback_context);
    }
    // Handle fault 
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
