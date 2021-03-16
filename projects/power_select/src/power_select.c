#include "power_select.h"
#include "gpio.h"
#include "log.h"

static PowerSelectStorage s_storage;

static bool s_measure = false;

// Gpio settings for sense pins
static const GpioSettings SENSE_SETTINGS = {
  GPIO_DIR_IN,
  GPIO_STATE_LOW,
  GPIO_RES_NONE,
  GPIO_ALTFN_ANALOG,
};

static const GpioAddress VOLTAGE_MEASUREMENT_PINS[NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_VSENSE_ADDR,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_VSENSE_ADDR,
  [POWER_SELECT_PWR_SUP] = POWER_SELECT_PWR_SUP_VSENSE_ADDR,
};

static const GpioAddress CURRENT_MEASUREMENT_PINS[NUM_POWER_SELECT_CURRENT_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_ISENSE_ADDR,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_ISENSE_ADDR,
  [POWER_SELECT_PWR_SUP] = POWER_SELECT_PWR_SUP_ISENSE_ADDR,
};

static const GpioAddress TEMP_MEASUREMENT_PINS[NUM_POWER_SELECT_TEMP_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_TEMP_ADDR,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_TEMP_ADDR,
};

static const GpioAddress VALID_PINS[NUM_POWER_SELECT_VALID_PINS] = {
  POWER_SELECT_AUX_VALID_ADDR,
  POWER_SELECT_DCDC_VALID_ADDR,
  POWER_SELECT_PWR_SUP_VALID_ADDR,
};

static const uint16_t MAX_VOLTAGES[NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_MAX_VOLTAGE_MV,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_MAX_VOLTAGE_MV,
  [POWER_SELECT_PWR_SUP] = POWER_SELECT_PWR_SUP_MAX_VOLTAGE_MV,
};

static const uint16_t MAX_CURRENTS[NUM_POWER_SELECT_CURRENT_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_MAX_CURRENT_MA,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_MAX_CURRENT_MA,
  [POWER_SELECT_PWR_SUP] = POWER_SELECT_PWR_SUP_MAX_CURRENT_MA,
};

// Broadcast the fault bitset
void prv_broadcast_fault(void) {
  CAN_TRANSMIT_POWER_SELECT_FAULT((uint64_t)s_storage.fault_bitset);
}
// Broadcast sense measurements from storage.
static StatusCode prv_broadcast_measurements(void) {
  StatusCode status = CAN_TRANSMIT_AUX_BATTERY_STATUS_MAIN_POWER_VOLTAGE(
      s_storage.voltages[POWER_SELECT_AUX], s_storage.currents[POWER_SELECT_AUX],
      s_storage.temps[POWER_SELECT_AUX], s_storage.voltages[POWER_SELECT_PWR_SUP]);
  status_ok_or_return(CAN_TRANSMIT_DCDC_BATTERY_STATUS_MAIN_POWER_CURRENT(
      s_storage.voltages[POWER_SELECT_DCDC], s_storage.currents[POWER_SELECT_DCDC],
      s_storage.temps[POWER_SELECT_DCDC], s_storage.currents[POWER_SELECT_PWR_SUP]));
  return status;
}

// Read current, voltage, and temp measurements to storage
// Note to self: might be easier to store these values as floats instead
// and then cast them to uint16s when sending over CAN
void prv_periodic_measure(SoftTimerId timer_id, void *context) {
  if (s_measure == false) {
    LOG_DEBUG("Stopping periodic measure\n");
    return;
  }
  LOG_DEBUG("Reading measurements...\n");
  AdcChannel sense_channel = NUM_ADC_CHANNELS;
  LOG_DEBUG("Note: 0 = AUX, 1 = DCDC, 2 = PWR SUP; valid pins active-low\n");

  for (uint8_t i = 0; i < NUM_POWER_SELECT_VALID_PINS; i++) {
    GpioState state = GPIO_STATE_LOW;
    gpio_get_state(&VALID_PINS[i], &state);
    // Valid pins are active-low
    if (state == GPIO_STATE_LOW) {
      s_storage.valid_bitset |= 1 << i;
    } else {
      s_storage.valid_bitset &= ~(1 << i);
    }
    LOG_DEBUG("Valid pin %d: %d\n", i, state == GPIO_STATE_HIGH);
  }

  float temp_reading = 0;
  for (uint32_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    // Only measure + store the value if the input pin is valid
    if (s_storage.valid_bitset & 1 << i) {
      adc_read_converted_pin(VOLTAGE_MEASUREMENT_PINS[i], &s_storage.voltages[i]);
      // convert
      temp_reading = s_storage.voltages[i];
      temp_reading /= POWER_SELECT_VSENSE_SCALING;
      temp_reading *= V_TO_MV;
      s_storage.voltages[i] = (uint16_t)temp_reading;

      // Check for fault, clear fault otherwise
      PowerSelectFault fault = i;
      if (s_storage.voltages[i] > MAX_VOLTAGES[i]) {
        s_storage.fault_bitset |= 1 << fault;
        prv_broadcast_fault();
      } else {
        s_storage.fault_bitset &= ~(1 << fault);
      }
    } else {
      s_storage.voltages[i] = 0;
    }
    LOG_DEBUG("Voltage %d: %d\n", (int)i, (int)s_storage.voltages[i]);
  }
  for (uint8_t i = 0; i < NUM_POWER_SELECT_CURRENT_MEASUREMENTS; i++) {
    // Only measure + store the value if the input pin is valid
    if (s_storage.valid_bitset & 1 << i) {
      adc_read_converted_pin(CURRENT_MEASUREMENT_PINS[i], &s_storage.currents[i]);
      // convert
      temp_reading = s_storage.currents[i];
      temp_reading *= A_TO_MA;
      temp_reading /= POWER_SELECT_ISENSE_SCALING;
      s_storage.currents[i] = (uint16_t)temp_reading;

      // check for fault
      PowerSelectFault fault = i + NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS;
      if (s_storage.currents[i] > MAX_CURRENTS[i]) {
        s_storage.fault_bitset |= 1 << fault;
        prv_broadcast_fault();
        // note: should probably broadcast this at the end of this cycle 
        // so if there's multiple faults we don't broadcast them multiple times
      } else {
        s_storage.fault_bitset &= ~(1 << fault);
      }
    } else {
      s_storage.currents[i] = 0;
    }
    LOG_DEBUG("Current %d: %d\n", (int)i, (int)s_storage.currents[i]);
  }
  for (uint8_t i = 0; i < NUM_POWER_SELECT_TEMP_MEASUREMENTS; i++) {
    uint16_t temp = 0;
    adc_read_converted_pin(TEMP_MEASUREMENT_PINS[i], &temp);

    // just using the old power selection thermistor functions for now.
    // pretty sure temp_to_res should be named voltage_to_res
    s_storage.temps[i] = (int32_t)resistance_to_temp(voltage_to_res(temp));
    LOG_DEBUG("Temp %d: %d\n", (int)i, s_storage.temps[i]);
  }

  LOG_DEBUG("Send measurements result: %d\n", prv_broadcast_measurements());
  soft_timer_start(POWER_SELECT_MEASUREMENT_INTERVAL_US, prv_periodic_measure, &s_storage,
                   &timer_id);
}

// Initialize all sense pins as ADC
static StatusCode prv_init_sense_pins(void) {
  for (uint32_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    status_ok_or_return(gpio_init_pin(&VOLTAGE_MEASUREMENT_PINS[i], &SENSE_SETTINGS));
    status_ok_or_return(adc_set_channel_pin(VOLTAGE_MEASUREMENT_PINS[i], true));
  }

  for (uint32_t i = 0; i < NUM_POWER_SELECT_CURRENT_MEASUREMENTS; i++) {
    status_ok_or_return(gpio_init_pin(&CURRENT_MEASUREMENT_PINS[i], &SENSE_SETTINGS));
    status_ok_or_return(adc_set_channel_pin(CURRENT_MEASUREMENT_PINS[i], true));
  }

  for (uint32_t i = 0; i < NUM_POWER_SELECT_TEMP_MEASUREMENTS; i++) {
    status_ok_or_return(gpio_init_pin(&TEMP_MEASUREMENT_PINS[i], &SENSE_SETTINGS));
    status_ok_or_return(adc_set_channel_pin(TEMP_MEASUREMENT_PINS[i], true));
  }

  return STATUS_CODE_OK;
}

void prv_handle_fault_it(const GpioAddress *address, void *context) {
  LOG_DEBUG("DCDC FAULT\n");
  // should figure out a way to clear this fault in future
  s_storage.fault_bitset |= 1 << POWER_SELECT_LTC_FAULT;
  prv_broadcast_fault();
}

// DCDC_FAULT pin goes high on fault
static StatusCode prv_init_fault_pin(void) {
  const GpioSettings settings = {
    GPIO_DIR_IN,
  };
  GpioAddress pin = POWER_SELECT_DCDC_FAULT_ADDR;
  status_ok_or_return(gpio_init_pin(&pin, &settings));

  const InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_it_register_interrupt(&pin, &it_settings, INTERRUPT_EDGE_RISING, prv_handle_fault_it,
                                    &s_storage);
  return STATUS_CODE_OK; // TEMP CHANGE DO NOT LEAVE IN
}

// Initialize valid pins (mainly for debugging purposes)
static StatusCode prv_init_valid_pins(void) {
  const GpioSettings settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_NONE,
  };

  for (uint8_t i = 0; i < NUM_POWER_SELECT_VALID_PINS; i++) {
    status_ok_or_return(gpio_init_pin(&VALID_PINS[i], &settings));
  }
  return STATUS_CODE_OK;
}

StatusCode power_select_init(void) {
  status_ok_or_return(prv_init_sense_pins());
  status_ok_or_return(prv_init_fault_pin());
  status_ok_or_return(prv_init_valid_pins());

  // Initialize SHDN pin (active low)
  GpioSettings shdn_settings = {
    GPIO_DIR_OUT,
    GPIO_STATE_HIGH,
    GPIO_RES_NONE,
    GPIO_ALTFN_NONE,
  };

  GpioAddress shdn_pin = POWER_SELECT_LTC_SHDN_ADDR;
  // Note: not using this atm
  status_ok_or_return(gpio_init_pin(&shdn_pin, &shdn_settings));

  return STATUS_CODE_OK;
}

// Start periodically measuring from the pins
StatusCode power_select_start(
    void) {  // do we even need this? or is it fine if we just expose prv_periodic_measure
  s_measure = true;
  prv_periodic_measure(SOFT_TIMER_INVALID_TIMER, &s_storage);
  
  return STATUS_CODE_OK;
}

void power_select_stop(void) {
  s_measure = false;
}

uint16_t power_select_get_fault_bitset(void) {
  return s_storage.fault_bitset;
}

uint8_t power_select_get_valid_bitset(void) {
  return s_storage.valid_bitset;
}

PowerSelectStorage power_select_get_storage(void) {
  return s_storage;
}
