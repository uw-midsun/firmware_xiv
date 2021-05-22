#include "power_select.h"

static PowerSelectStorage s_storage;

// Populate global arrs
const GpioAddress g_power_select_voltage_pins[NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_VSENSE_ADDR,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_VSENSE_ADDR,
  [POWER_SELECT_PWR_SUP] = POWER_SELECT_PWR_SUP_VSENSE_ADDR,
};

const GpioAddress g_power_select_current_pins[NUM_POWER_SELECT_CURRENT_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_ISENSE_ADDR,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_ISENSE_ADDR,
  [POWER_SELECT_PWR_SUP] = POWER_SELECT_PWR_SUP_ISENSE_ADDR,
};

const GpioAddress g_power_select_temp_pins[NUM_POWER_SELECT_TEMP_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_TEMP_ADDR,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_TEMP_ADDR,
};

const GpioAddress g_power_select_valid_pins[NUM_POWER_SELECT_VALID_PINS] = {
  POWER_SELECT_AUX_VALID_ADDR,
  POWER_SELECT_DCDC_VALID_ADDR,
  POWER_SELECT_PWR_SUP_VALID_ADDR,
};

const uint16_t g_power_select_max_voltages[NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_MAX_VOLTAGE_MV,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_MAX_VOLTAGE_MV,
  [POWER_SELECT_PWR_SUP] = POWER_SELECT_PWR_SUP_MAX_VOLTAGE_MV,
};

const uint16_t g_power_select_max_currents[NUM_POWER_SELECT_CURRENT_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_MAX_CURRENT_MA,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_MAX_CURRENT_MA,
  [POWER_SELECT_PWR_SUP] = POWER_SELECT_PWR_SUP_MAX_CURRENT_MA,
};

// Broadcast the fault bitset and turn off/on the LTC as required
static void prv_handle_fault(void) {
  // Turn off/on LTC depending on fault status
  GpioAddress pin = POWER_SELECT_LTC_SHDN_ADDR;
  if ((s_storage.fault_bitset & POWER_SELECT_LTC_DISABLE_FAULT_MASK) == 0) {
    // Turn back on if not already
    gpio_set_state(&pin, GPIO_STATE_HIGH);
  } else {
    // Fault, turn off LTC
    gpio_set_state(&pin, GPIO_STATE_LOW);
  }
  CAN_TRANSMIT_POWER_SELECT_FAULT((uint64_t)s_storage.fault_bitset);
}

// Broadcast sense measurements from storage.
static StatusCode prv_broadcast_measurements(void) {
  StatusCode status = CAN_TRANSMIT_AUX_STATUS_MAIN_VOLTAGE(
      s_storage.voltages[POWER_SELECT_AUX], s_storage.currents[POWER_SELECT_AUX],
      s_storage.temps[POWER_SELECT_AUX], s_storage.voltages[POWER_SELECT_PWR_SUP]);
  status_ok_or_return(CAN_TRANSMIT_DCDC_STATUS_MAIN_CURRENT(
      s_storage.voltages[POWER_SELECT_DCDC], s_storage.currents[POWER_SELECT_DCDC],
      s_storage.temps[POWER_SELECT_DCDC], s_storage.currents[POWER_SELECT_PWR_SUP]));
  return status;
}

// Helper function to read, output, and store pin validity
static void prv_read_valid(void) {
  for (uint8_t i = 0; i < NUM_POWER_SELECT_VALID_PINS; i++) {
    GpioState state = GPIO_STATE_LOW;
    gpio_get_state(&g_power_select_valid_pins[i], &state);
    // Valid pins are active-low, bitset is active-high
    if (state == GPIO_STATE_LOW) {
      s_storage.valid_bitset |= (1 << i);
    } else {
      s_storage.valid_bitset &= ~(1 << i);
    }
    LOG_DEBUG("Valid pin %d: %d\n", i, state == GPIO_STATE_HIGH);
  }
}

// Helper function to read, output, and store voltage values
static void prv_read_voltages(void) {
  float temp_reading = 0;
  for (uint8_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    // Only measure + store the value if the input pin is valid
    if (s_storage.valid_bitset & (1 << i)) {
      adc_read_converted_pin(g_power_select_voltage_pins[i], &s_storage.voltages[i]);
      // Convert
      temp_reading = s_storage.voltages[i];
      temp_reading *= V_TO_MV;
      temp_reading /= POWER_SELECT_VSENSE_SCALING;
      s_storage.voltages[i] = (uint16_t)temp_reading;

      // Check for fault, clear fault otherwise
      PowerSelectFault fault = i;
      if (s_storage.voltages[i] > g_power_select_max_voltages[i]) {
        s_storage.fault_bitset |= (1 << fault);
      } else {
        s_storage.fault_bitset &= ~(1 << fault);
      }
    } else {
      s_storage.voltages[i] = 0;
    }
    LOG_DEBUG("Voltage %" PRIu8 ": %" PRIu16 " mV\n", i, s_storage.voltages[i]);
  }
}

// Helper function to read, output, and store temperature values
static void prv_read_temps(void) {
  for (uint8_t i = 0; i < NUM_POWER_SELECT_TEMP_MEASUREMENTS; i++) {
    uint16_t temp = 0;
    adc_read_converted_pin(g_power_select_temp_pins[i], &temp);

    s_storage.temps[i] = (int32_t)resistance_to_temp(voltage_to_res(temp));
    LOG_DEBUG("Temp %" PRIu8 ": %" PRIi32 " C\n", i, s_storage.temps[i]);
  }
}

// Helper function to read, output, and store current values
static void prv_read_currents(void) {
  float temp_reading = 0;
  for (uint8_t i = 0; i < NUM_POWER_SELECT_CURRENT_MEASUREMENTS; i++) {
    // Only measure + store the value if the input pin is valid
    if (s_storage.valid_bitset & (1 << i)) {
      adc_read_converted_pin(g_power_select_current_pins[i], &s_storage.currents[i]);
      // Convert
      temp_reading = s_storage.currents[i];
      temp_reading *= A_TO_MA;
      temp_reading /= POWER_SELECT_ISENSE_SCALING;
      s_storage.currents[i] = (uint16_t)temp_reading;

      // Check for fault, clear fault otherwise
      PowerSelectFault fault = i + NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS;
      if (s_storage.currents[i] > g_power_select_max_currents[i]) {
        s_storage.fault_bitset |= (1 << fault);
      } else {
        s_storage.fault_bitset &= ~(1 << fault);
      }
    } else {
      s_storage.currents[i] = 0;
    }
    LOG_DEBUG("Current %" PRIu8 ": %" PRIu16 " mA\n", i, s_storage.currents[i]);
  }
}

// Read current, voltage, and temp measurements to storage
static void prv_periodic_measure(SoftTimerId timer_id, void *context) {
  LOG_DEBUG("Reading measurements...\n");
  LOG_DEBUG("Note: 0 = AUX, 1 = DCDC, 2 = PWR SUP; valid pins active-low\n");

  // Read values
  prv_read_valid();

  prv_read_voltages();

  prv_read_currents();

  prv_read_temps();

  // Broadcast
  prv_broadcast_measurements();

  prv_handle_fault();

  soft_timer_start(s_storage.interval_us, prv_periodic_measure, &s_storage, &s_storage.timer_id);
}

// Initialize all sense pins as ADC
static StatusCode prv_init_sense_pins(void) {
  GpioSettings sense_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  for (uint32_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    status_ok_or_return(gpio_init_pin(&g_power_select_voltage_pins[i], &sense_settings));
    status_ok_or_return(adc_set_channel_pin(g_power_select_voltage_pins[i], true));
  }

  for (uint32_t i = 0; i < NUM_POWER_SELECT_CURRENT_MEASUREMENTS; i++) {
    status_ok_or_return(gpio_init_pin(&g_power_select_current_pins[i], &sense_settings));
    status_ok_or_return(adc_set_channel_pin(g_power_select_current_pins[i], true));
  }

  for (uint32_t i = 0; i < NUM_POWER_SELECT_TEMP_MEASUREMENTS; i++) {
    status_ok_or_return(gpio_init_pin(&g_power_select_temp_pins[i], &sense_settings));
    status_ok_or_return(adc_set_channel_pin(g_power_select_temp_pins[i], true));
  }

  return STATUS_CODE_OK;
}

static void prv_handle_fault_it(const GpioAddress *address, void *context) {
  // Pin high on fault, low when cleared
  GpioState state = GPIO_STATE_HIGH;
  GpioAddress pin = POWER_SELECT_DCDC_FAULT_ADDR;
  gpio_get_state(&pin, &state);
  if (state == GPIO_STATE_HIGH) {
    LOG_DEBUG("DCDC FAULT\n");
    s_storage.fault_bitset |= (1 << POWER_SELECT_DCDC_FAULT);
  } else {
    LOG_DEBUG("DCDC fault cleared\n");
    s_storage.fault_bitset &= ~(1 << POWER_SELECT_DCDC_FAULT);
  }

  prv_handle_fault();
}

// DCDC_FAULT pin goes high on fault
static StatusCode prv_init_fault_pin(void) {
  const GpioSettings settings = { .direction = GPIO_DIR_IN,
                                  .state = GPIO_STATE_LOW,
                                  .alt_function = GPIO_ALTFN_NONE,
                                  .resistor = GPIO_RES_NONE };
  GpioAddress pin = POWER_SELECT_DCDC_FAULT_ADDR;
  status_ok_or_return(gpio_init_pin(&pin, &settings));

  const InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_it_register_interrupt(&pin, &it_settings, INTERRUPT_EDGE_RISING_FALLING, prv_handle_fault_it,
                             NULL);
  return STATUS_CODE_OK;
}

// Initialize valid pins
static StatusCode prv_init_valid_pins(void) {
  const GpioSettings settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  for (uint8_t i = 0; i < NUM_POWER_SELECT_VALID_PINS; i++) {
    status_ok_or_return(gpio_init_pin(&g_power_select_valid_pins[i], &settings));
  }
  return STATUS_CODE_OK;
}

StatusCode power_select_init(void) {
  status_ok_or_return(prv_init_sense_pins());
  status_ok_or_return(prv_init_fault_pin());
  status_ok_or_return(prv_init_valid_pins());

  // Initialize SHDN pin (active low)
  GpioSettings shdn_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  GpioAddress shdn_pin = POWER_SELECT_LTC_SHDN_ADDR;
  status_ok_or_return(gpio_init_pin(&shdn_pin, &shdn_settings));
  gpio_set_state(&shdn_pin, GPIO_STATE_HIGH);

  s_storage.timer_id = SOFT_TIMER_INVALID_TIMER;
  s_storage.measurement_in_progress = false;
  s_storage.interval_us = POWER_SELECT_MEASUREMENT_INTERVAL_US;

  return STATUS_CODE_OK;
}

StatusCode power_select_start(uint32_t interval_us) {
  s_storage.interval_us = interval_us;
  if (s_storage.measurement_in_progress) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
  s_storage.measurement_in_progress = true;
  prv_periodic_measure(s_storage.timer_id, &s_storage);
  return STATUS_CODE_OK;
}

bool power_select_stop(void) {
  bool status = soft_timer_cancel(s_storage.timer_id);
  s_storage.measurement_in_progress = false;
  s_storage.timer_id = SOFT_TIMER_INVALID_TIMER;
  return status;
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
