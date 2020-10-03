#include "bts_7200_current_sense.h"
#include <stddef.h>
#include "log.h"

#define STM32_GPIO_STATE_SELECT_OUT_0 GPIO_STATE_LOW
#define STM32_GPIO_STATE_SELECT_OUT_1 GPIO_STATE_HIGH
#define PCA9539R_GPIO_STATE_SELECT_OUT_0 PCA9539R_GPIO_STATE_LOW
#define PCA9539R_GPIO_STATE_SELECT_OUT_1 PCA9539R_GPIO_STATE_HIGH

// Avoid starting soft timer for the same fault multiple times 
static bool s_handling_input_0_fault = false; 
static bool s_handling_input_1_fault = false;

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

// Set s_handling_input_0_fault to false
static void prv_bts_7200_fault_handler_set_handling_input_0_false_cb(SoftTimerId timer_id, void *context) {
  s_handling_input_0_fault = false;
} 

/*
// Set s_handling_input_1_fault to false
static void prv_bts_7200_fault_handler_set_handling_input_1_false_cb(SoftTimerId timer_id, void *context) {
  s_handling_input_1_fault = false;
}*/

// Callback for soft timer in prv_bts_7200_handle_fault to re-enable IN0
static void prv_bts_7200_fault_handler_enable_0_cb(SoftTimerId timer_id, void* context) {
  Bts7200Storage *storage = context;
  bts_7200_enable_output_0(storage);
  prv_bts_7200_fault_handler_set_handling_input_0_false_cb(timer_id, &storage);
}

/*
// Callback for soft timer in prv_bts_7200_handle_fault to re-enable IN1
static void prv_bts_7200_fault_handler_enable_1_cb(SoftTimerId timer_id, void *context) {
  Bts7200Storage *storage = context;
  bts_7200_enable_output_1(storage);
  prv_bts_7200_fault_handler_set_handling_input_1_false_cb(timer_id, &storage);
}*/

static StatusCode prv_bts_7200_handle_fault(Bts7200Storage *storage, bool fault0, bool fault1) {
  //LOG_DEBUG("Handling fault\n");
  if (fault0) {
    //LOG_DEBUG("gets into fault0\n");
    // Storage for previous state of IN pin
    bool prev_gpio_state = 0;
    prev_gpio_state = bts_7200_get_output_0_enabled(storage);
    // Pull IN0 low + wait for BTS7200_FAULT_RESTART_DELAY_US
    status_ok_or_return(bts_7200_disable_output_0(storage));
    //LOG_DEBUG("up to here\n");
    //LOG_DEBUG("prev_gpio_state: %d, s_handling_input_0_fault: %d\n", prev_gpio_state, s_handling_input_0_fault);
    // Only start a new soft timer if the timer doesn't currently exist
    if(soft_timer_remaining_time(BTS7200_FAULT_INPUT_0_TIMER) == 0) {
      if(prev_gpio_state) {
        soft_timer_start(BTS7200_FAULT_RESTART_DELAY_US, prv_bts_7200_fault_handler_enable_0_cb, storage, BTS7200_FAULT_INPUT_0_TIMER);
      }
      else {
        soft_timer_start(BTS7200_FAULT_RESTART_DELAY_US, prv_bts_7200_fault_handler_set_handling_input_0_false_cb, storage, BTS7200_FAULT_INPUT_0_TIMER);
      }
    }
    /*
    if (prev_gpio_state && !s_handling_input_0_fault) {
      //LOG_DEBUG("gets into first if\n");
      s_handling_input_0_fault = true;
      soft_timer_start(BTS7200_FAULT_RESTART_DELAY_US, prv_bts_7200_fault_handler_enable_0_cb, storage, BTS7200_FAULT_INPUT_0_TIMER);
    } else if (!s_handling_input_0_fault) {
      //LOG_DEBUG("gets into second if\n");
      soft_timer_start(BTS7200_FAULT_RESTART_DELAY_US, prv_bts_7200_fault_handler_set_handling_input_0_false_cb, storage, BTS7200_FAULT_INPUT_0_TIMER);
      s_handling_input_0_fault = true;
    }
    */
  }
  /*
  if (fault1) {
    //LOG_DEBUG("gets into fault1\n");
    // Storage for previous state of IN pin
    bool prev_gpio_state = 0;
    prev_gpio_state = bts_7200_get_output_1_enabled(storage);

    // Pull IN1 low + wait for BTS7200_FAULT_RESTART_DELAY_US
    status_ok_or_return(bts_7200_disable_output_1(storage));

    // If output was enabled before fault, re-enable after delay
    if (prev_gpio_state && !s_handling_input_1_fault) {
      s_handling_input_1_fault = true;
      soft_timer_start(BTS7200_FAULT_RESTART_DELAY_US, prv_bts_7200_fault_handler_enable_1_cb, storage, NULL);
    } else if (!s_handling_input_1_fault) {
      s_handling_input_1_fault = true;
      soft_timer_start(BTS7200_FAULT_RESTART_DELAY_US, prv_bts_7200_fault_handler_set_handling_input_1_false_cb, storage, NULL);   
    }
    //LOG_DEBUG("FAULT 1 UNUSED ATM\n");
  }*/
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
  ////LOG_DEBUG("Getting measurement\n");
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
  //bool fault0 = BTS7200_IS_MEASUREMENT_FAULT(*meas0);
  //bool fault1 = BTS7200_IS_MEASUREMENT_FAULT(*meas1);
  /*
  if (fault0 || fault1) {
    ////LOG_DEBUG("calling fault cb\n");
    //storage->fault_callback(fault0, fault1, storage->fault_callback_context);
    //prv_bts_7200_handle_fault(storage, fault0, fault1);
  }*/

  if(false) {
    prv_bts_7200_handle_fault(storage, false, false);
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
