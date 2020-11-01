#include "power_select.h"
#include "gpio.h"
#include "log.h"

static PowerSelectStorage s_storage;

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
void prv_periodic_measure(SoftTimerId timer_id, void *context) {
  LOG_DEBUG("Reading measurements...\n");
  AdcChannel sense_channel = NUM_ADC_CHANNELS;

  for (uint32_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    (adc_get_channel(VOLTAGE_MEASUREMENT_PINS[i], &sense_channel));
    (adc_read_converted(sense_channel, &s_storage.voltages[i]));
  }
  for (uint8_t i = 0; i < NUM_POWER_SELECT_CURRENT_MEASUREMENTS; i++) {
    (adc_get_channel(CURRENT_MEASUREMENT_PINS[i], &sense_channel));
    (adc_read_converted(sense_channel, &s_storage.currents[i]));
  }
  for (uint8_t i = 0; i < NUM_POWER_SELECT_TEMP_MEASUREMENTS; i++) {
    (adc_get_channel(TEMP_MEASUREMENT_PINS[i], &sense_channel));
    (adc_read_converted(sense_channel, &s_storage.temps[i]));
  }

  prv_broadcast_measurements();
  soft_timer_start(POWER_SELECT_MEASUREMENT_INTERVAL_US, prv_periodic_measure, &s_storage,
                   &timer_id);
}

// Initialize all sense pins as ADC
static StatusCode prv_init_sense_pins(void) {
  AdcChannel sense_channel = NUM_ADC_CHANNELS;

  for (uint32_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    status_ok_or_return(gpio_init_pin(&VOLTAGE_MEASUREMENT_PINS[i], &SENSE_SETTINGS));
    status_ok_or_return(adc_get_channel(VOLTAGE_MEASUREMENT_PINS[i], &sense_channel));
    status_ok_or_return(adc_set_channel(sense_channel, true));
    sense_channel = NUM_ADC_CHANNELS;
  }

  for (uint32_t i = 0; i < NUM_POWER_SELECT_CURRENT_MEASUREMENTS; i++) {
    status_ok_or_return(gpio_init_pin(&CURRENT_MEASUREMENT_PINS[i], &SENSE_SETTINGS));
    status_ok_or_return(adc_get_channel(CURRENT_MEASUREMENT_PINS[i], &sense_channel));
    status_ok_or_return(adc_set_channel(sense_channel, true));
    sense_channel = NUM_ADC_CHANNELS;
  }

  for (uint32_t i = 0; i < NUM_POWER_SELECT_TEMP_MEASUREMENTS; i++) {
    status_ok_or_return(gpio_init_pin(&TEMP_MEASUREMENT_PINS[i], &SENSE_SETTINGS));
    status_ok_or_return(adc_get_channel(TEMP_MEASUREMENT_PINS[i], &sense_channel));
    status_ok_or_return(adc_set_channel(sense_channel, true));
    sense_channel = NUM_ADC_CHANNELS;
  }

  return STATUS_CODE_OK;
}

void prv_handle_fault_it(const GpioAddress *address, void *context) {
  LOG_DEBUG("DCDC FAULT\n");
  // send a fault CAN message + eventually pull SHDN low 
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

  return gpio_it_register_interrupt(&pin, &it_settings, INTERRUPT_EDGE_RISING, prv_handle_fault_it, &s_storage);
}

// Initialize valid pins (mainly for debugging purposes)
static StatusCode prv_init_valid_pins(void) {
  const GpioSettings settings = {
    GPIO_DIR_IN, 
    GPIO_STATE_LOW, 
    GPIO_RES_NONE, 
    GPIO_ALTFN_NONE,
  };

  GpioAddress pin = POWER_SELECT_VALID1_ADDR;
  status_ok_or_return(gpio_init_pin(&pin, &settings));
  GpioAddress pin2 = POWER_SELECT_VALID2_ADDR;
  status_ok_or_return(gpio_init_pin(&pin2, &settings));
  GpioAddress pin3 = POWER_SELECT_VALID3_ADDR;
  return gpio_init_pin(&pin3, &settings);
}

StatusCode power_select_init(void) {
  status_ok_or_return(prv_init_sense_pins());
  status_ok_or_return(prv_init_fault_pin());
  status_ok_or_return(prv_init_valid_pins());
  return STATUS_CODE_OK;
}

// Start periodically measuring from the pins
StatusCode power_select_start(
    void) {  // do we even need this? or is it fine if we just expose prv_periodic_measure
  prv_periodic_measure(SOFT_TIMER_INVALID_TIMER, &s_storage);
  return STATUS_CODE_OK;
}
