#include "begin_sequence.h"

#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "can_transmit.h"
#include "charger_controller.h"
#include "charger_events.h"
#include "control_pilot.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "status.h"

static const GpioAddress s_cp_select = CONTROL_PILOT_SEL_PIN;
static const GpioAddress s_charger_sense = CHARGER_SENSE_PIN;
static const GpioAddress s_relay_en = RELAY_EN_PIN;
static const GpioAddress s_load_sw_en = LOAD_SW_EN_PIN;

static StatusCode prv_sequence(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  GpioState state = NUM_GPIO_STATES;

  // 1. Ensure charger is on
  gpio_get_state(&s_charger_sense, &state);
  if (state != GPIO_STATE_HIGH) {
    CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_CHARGER_OFF);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  // 2. Get control pilot PWM reading
  uint16_t cp_pwm_ret = control_pilot_get_current();

  // 3. Set relay and load switch state
  gpio_set_state(&s_relay_en, GPIO_STATE_HIGH);
  gpio_set_state(&s_load_sw_en, GPIO_STATE_HIGH);

  // 4. Enable charging via control pilot
  gpio_set_state(&s_cp_select, GPIO_STATE_HIGH);

  // 5. Activate charger
  charger_controller_activate(cp_pwm_ret);

  return STATUS_CODE_OK;
}

void begin_sequence_process_event(const Event *e) {
  if (e->id == CHARGER_CHARGE_EVENT_BEGIN) {
    CAN_TRANSMIT_REQUEST_TO_CHARGE();
  }
}

StatusCode begin_sequence_init() {
  GpioSettings settings = {
    .direction = GPIO_DIR_OUT,       //
    .state = GPIO_STATE_LOW,         //
    .resistor = GPIO_RES_NONE,       //
    .alt_function = GPIO_ALTFN_NONE  //
  };
  gpio_init_pin(&s_cp_select, &settings);
  gpio_init_pin(&s_relay_en, &settings);
  gpio_init_pin(&s_load_sw_en, &settings);

  settings.direction = GPIO_DIR_IN;
  gpio_init_pin(&s_charger_sense, &settings);

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_ALLOW_CHARGING, prv_sequence, NULL);
  control_pilot_init();

  return STATUS_CODE_OK;
}
