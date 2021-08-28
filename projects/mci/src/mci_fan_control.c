#include "mci_fan_control.h"

#include "gpio.h"
#include "gpio_it.h"

static MciFanControlStorage s_storage = { 0 };

// Enable pin
static const GpioAddress s_en_addr = MCI_FAN_EN_ADDR;

// Thermistor pins
const GpioAddress g_therm_addrs[NUM_MCI_FAN_CONTROL_THERMS] = {
  [MCI_THERM_DISCHARGE_OVERTEMP] = MCI_DISCHARGE_OVERTEMP_ADDR,
  [MCI_THERM_PRECHARGE_OVERTEMP] = MCI_PRECHARGE_OVERTEMP_ADDR,
  [MCI_THERM_Q1_OVERTEMP] = MCI_Q1_OVERTEMP_ADDR,
  [MCI_THERM_Q3_OVERTEMP] = MCI_Q3_OVERTEMP_ADDR,
};

// Store fault indices to pass as context to interrupt callback
static const uint8_t s_therm_indices[NUM_MCI_FAN_CONTROL_THERMS] = {
  [MCI_THERM_DISCHARGE_OVERTEMP] = MCI_THERM_DISCHARGE_OVERTEMP,
  [MCI_THERM_PRECHARGE_OVERTEMP] = MCI_THERM_PRECHARGE_OVERTEMP,
  [MCI_THERM_Q1_OVERTEMP] = MCI_THERM_Q1_OVERTEMP,
  [MCI_THERM_Q3_OVERTEMP] = MCI_THERM_Q3_OVERTEMP,
};

// Handle a fault interrupt.
static void prv_handle_therm_it(const GpioAddress *address, void *context) {
  // Context holds the index of the fault pin
  uint8_t index = *((uint8_t *)(context));

  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(&g_therm_addrs[index], &state);

  if (state == GPIO_STATE_HIGH) {
    // Set fault
    s_storage.fault_bitset |= 1 << index;
  } else {
    // Clear fault
    s_storage.fault_bitset &= ~(1 << index);
  }

  // Only call the fault callback if it exists
  if (s_storage.fault_cb) {
    s_storage.fault_cb(s_storage.fault_bitset, s_storage.fault_context);
  }
}

// Set up a thermistor pin for interrupts.
static StatusCode prv_configure_therm_it(GpioAddress *address, uint8_t pin_idx) {
  const GpioSettings settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE,
  };
  status_ok_or_return(gpio_init_pin(address, &settings));

  const InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  return gpio_it_register_interrupt(address, &it_settings, INTERRUPT_EDGE_RISING_FALLING,
                                    prv_handle_therm_it, &s_therm_indices[pin_idx]);
}

StatusCode mci_fan_control_init(MciFanControlSettings *settings) {
  // Fan initially off
  GpioSettings pin_settings = {
    .direction = GPIO_DIR_OUT,
    .alt_function = GPIO_ALTFN_NONE,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
  };

  status_ok_or_return(gpio_init_pin(&s_en_addr, &pin_settings));
  status_ok_or_return(mci_fan_set_state(MCI_FAN_STATE_OFF));

  s_storage.fault_cb = settings->fault_cb;
  s_storage.fault_context = settings->fault_context;

  // Configure all thermistor pins
  for (uint8_t i = 0; i < NUM_MCI_FAN_CONTROL_THERMS; i++) {
    status_ok_or_return(prv_configure_therm_it(&g_therm_addrs[i], i));
  }

  return STATUS_CODE_OK;
}

StatusCode mci_fan_set_state(MciFanState state) {
  GpioState gpio_state = (state == MCI_FAN_STATE_ON) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  return gpio_set_state(&s_en_addr, gpio_state);
}

uint8_t mci_fan_get_fault_bitset(void) {
  return s_storage.fault_bitset;
}
