/* PRECHARGE CONTROL
 * - receives can message for precharge
 * - begins precharge (sets gpio pin)
 *      - acks the precharge message (checks the gpio state after setting it, if it’s the same, ack
 * status OK)
 * - receives can message for discharge (power-off sequence, bms faults)
 *      - acks the message
 * - receives fault events for discharge(internally generated if the mci’s fault)
 * - sends precharge complete message when precharge is complete (interrupt)
 * - keeps a global state of the precharge status (the interrupt should be triggered on both rising
 * and falling)
 */

// Pin A9 is the pin to start precharge.
// Pin A10 is the pin to monitor precharge state.

/* Questions
 * - Are my pins right?
 * - What exactly is the expected pin behaviour?
 * - What is the begin precharge message?
 *    - A: POWER_ON_MAIN_SEQUENCE, data: uint8_t begin precharge in EE
 * - What is the discharge message?
 *    - A: power off sequence, not implemented yet
 * - Can I keep the precharge state in the motor controller storage, or should I use
 *       a static variable with a getter method?
 * - What is the precharge complete message?
 *    - A: POWER_MAIN_SEQUENCE_PRECHARGE_COMPLETED in EE
 */

#include "can.h"
#include "can_ack.h"
#include "gpio.h"
#include "gpio_it.h"
#include "status.h"

#include "motor_controller.h"

// THESE PINS ARE GOING TO BE CHANGED ONCE MCI REV5 IS OUTs
static const GpioSettings s_control_settings = {
  .direction = GPIO_DIR_OUT,
  .state = GPIO_STATE_LOW,
  .alt_function = GPIO_ALTFN_NONE,
  .resistor = GPIO_RES_NONE
};

static const GpioSettings s_monitor_settings = {
  .direction = GPIO_DIR_IN,
  .state = GPIO_STATE_LOW,
  .alt_function = GPIO_ALTFN_NONE,
  .resistor = GPIO_RES_NONE
};
static const InterruptSettings s_monitor_it_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL
};

void prv_monitor_int(void *context) {
  MotorControllerStorage *storage = context;
  GpioState monitor_state = GPIO_STATE_LOW;
  gpio_get_state(&storage->settings.precharge_monitor, &monitor_state);
  if (monitor_state == GPIO_STATE_LOW) {
    storage->precharge_state = MCI_PRECHARGE_DISCHARGED;
  } else if (monitor_state == GPIO_STATE_HIGH) {
    storage->precharge_state = MCI_PRECHARGE_CHARGED;
  }
}

StatusCode prv_precharge(void *context) {
  // make sure A10 is off
  GpioState monitor_state = GPIO_STATE_LOW;
  gpio_get_state(&precharge_monitor, &monitor_state);
  if (monitor_state != GPIO_STATE_LOW) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  // set A9 to on
  gpio_set_state(&precharge_control, GPIO_STATE_HIGH);
  // check A9 to make sure it's on
  GpioState control_state = GPIO_STATE_LOW;
  gpio_get_state(&precharge_control, &control_state);
  if (control_state != GPIO_STATE_HIGH) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return STATUS_CODE_OK;
}

StatusCode prv_discharge(void *context) {
  // set precharge_control to off
  MotorControllerStorage *storage = context;
  gpio_set_state(&storage->settings.precharge_control, GPIO_STATE_LOW);
  // check precharge_control to make sure it's discharged
  GpioState control_state = GPIO_STATE_LOW;
  gpio_get_state(&storage->settings.precharge_monitor, &control_state);
  if (control_state != GPIO_STATE_LOW) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return STATUS_CODE_OK;
}

void begin_precharge_rx(void *context) {
  // prv_precharge
  // ACK message
}

void discharge_rx(void *context) {
  // prv_discharge
  // ACK message
}

// gpio_init() and gpio_it_init() is required before this is called
void precharge_control_init(void *context) {
  MotorControllerStorage *storage = context;
  // setup gpio pin A9 for starting precharge
  gpio_init_pin(&storage->settings.precharge_control, &s_control_settings);

  // setup gpio pin A10 for monitoring precharge state via interrupt (INTERRUPT_EDGE_RISING_FALLING)
  gpio_init_pin(&storage->settings.precharge_monitor, &s_monitor_settings);
  gpio_it_register_interrupt(&storage->settings.precharge_monitor, 
                             &s_monitor_it_settings, 
                             INTERRUPT_EDGE_RISING_FALLING, prv_monitor_int, context);

  // setup handler for CAN begin precharge (with ack)
  // can_register_rx_handler(SYSTEM_CAN_MESSAGE_)
  
  // setup handler for CAN discharge (with ack)
  // setup handler for faults
}