#include "led_manager.h"

#include <stdbool.h>

#include "centre_console_events.h"
#include "centre_console_fault_reason.h"
#include "event_queue.h"
#include "mcp23008_gpio_expander.h"
#include "misc.h"
#include "status.h"

// The current list of LED management rules:
// * the BPS LED turns on when we receive a BPS fault and never turns off, since BPS faults
//   currently can never be cleared
// * the hazards LED mirrors the hazards instructions we give to PD (SYSTEM_CAN_MESSAGE_HAZARD)
// * the power LED turns on when we complete the main or aux power sequences and turns off when
//   we complete the power off sequence
// * the drive, reverse, neutral, and parking LEDs (collectively, the "drive state LEDs") are
//   mutually exclusive, and one is on depending on the state of the drive FSM

#define MCP23008_I2C_PORT I2C_PORT_2
#define MCP23008_I2C_ADDR 0x27

typedef void (*EventHandler)(Event *e);

static const Mcp23008GpioAddress s_led_to_address[NUM_CENTRE_CONSOLE_LEDS] = {
  [CENTRE_CONSOLE_LED_BPS] = { MCP23008_I2C_ADDR, 0 },
  [CENTRE_CONSOLE_LED_POWER] = { MCP23008_I2C_ADDR, 1 },
  [CENTRE_CONSOLE_LED_REVERSE] = { MCP23008_I2C_ADDR, 2 },
  [CENTRE_CONSOLE_LED_NEUTRAL] = { MCP23008_I2C_ADDR, 3 },
  [CENTRE_CONSOLE_LED_DRIVE] = { MCP23008_I2C_ADDR, 4 },
  [CENTRE_CONSOLE_LED_SPARE] = { MCP23008_I2C_ADDR, 5 },
  [CENTRE_CONSOLE_LED_PARKING] = { MCP23008_I2C_ADDR, 6 },
  [CENTRE_CONSOLE_LED_HAZARDS] = { MCP23008_I2C_ADDR, 7 },
};

static const CentreConsoleLed s_drive_state_leds[] = {
  CENTRE_CONSOLE_LED_DRIVE,
  CENTRE_CONSOLE_LED_REVERSE,
  CENTRE_CONSOLE_LED_NEUTRAL,
  CENTRE_CONSOLE_LED_PARKING,
};

static const CentreConsoleLed s_drive_fsm_output_events_to_leds[] = {
  [DRIVE_FSM_OUTPUT_EVENT_DRIVE] = CENTRE_CONSOLE_LED_DRIVE,
  [DRIVE_FSM_OUTPUT_EVENT_REVERSE] = CENTRE_CONSOLE_LED_REVERSE,
  [DRIVE_FSM_OUTPUT_EVENT_NEUTRAL] = CENTRE_CONSOLE_LED_NEUTRAL,
  [DRIVE_FSM_OUTPUT_EVENT_PARKING] = CENTRE_CONSOLE_LED_PARKING,
};

static void prv_set_led(CentreConsoleLed led, Mcp23008GpioState state) {
  mcp23008_gpio_set_state(&s_led_to_address[led], state);
}

static void prv_set_bps_led(Event *e) {
  FaultReason reason = { .raw = e->data };
  if (reason.fields.area == EE_CONSOLE_FAULT_AREA_BPS_HEARTBEAT) {
    prv_set_led(CENTRE_CONSOLE_LED_BPS, MCP23008_GPIO_STATE_HIGH);
  }
}

static void prv_set_hazards_led(Event *e) {
  prv_set_led(CENTRE_CONSOLE_LED_HAZARDS,
              e->id == HAZARD_EVENT_ON ? MCP23008_GPIO_STATE_HIGH : MCP23008_GPIO_STATE_LOW);
}

static void prv_set_power_led(Event *e) {
  if (e->id == POWER_MAIN_SEQUENCE_EVENT_COMPLETE || e->id == POWER_AUX_SEQUENCE_EVENT_COMPLETE) {
    prv_set_led(CENTRE_CONSOLE_LED_POWER, MCP23008_GPIO_STATE_HIGH);
  } else if (e->id == POWER_OFF_SEQUENCE_EVENT_COMPLETE) {
    prv_set_led(CENTRE_CONSOLE_LED_POWER, MCP23008_GPIO_STATE_LOW);
  }
}

static void prv_set_drive_state_leds(Event *e) {
  CentreConsoleLed led_to_enable = s_drive_fsm_output_events_to_leds[e->id];
  for (size_t i = 0; i < SIZEOF_ARRAY(s_drive_state_leds); i++) {
    CentreConsoleLed led = s_drive_state_leds[i];
    prv_set_led(led, led == led_to_enable ? MCP23008_GPIO_STATE_HIGH : MCP23008_GPIO_STATE_LOW);
  }
}

static const EventHandler s_event_to_handler[NUM_CENTRE_CONSOLE_EVENTS] = {
  [CENTRE_CONSOLE_POWER_EVENT_FAULT] = prv_set_bps_led,
  [HAZARD_EVENT_ON] = prv_set_hazards_led,
  [HAZARD_EVENT_OFF] = prv_set_hazards_led,
  [POWER_MAIN_SEQUENCE_EVENT_COMPLETE] = prv_set_power_led,
  [POWER_AUX_SEQUENCE_EVENT_COMPLETE] = prv_set_power_led,
  [POWER_OFF_SEQUENCE_EVENT_COMPLETE] = prv_set_power_led,
  [DRIVE_FSM_OUTPUT_EVENT_DRIVE] = prv_set_drive_state_leds,
  [DRIVE_FSM_OUTPUT_EVENT_REVERSE] = prv_set_drive_state_leds,
  [DRIVE_FSM_OUTPUT_EVENT_NEUTRAL] = prv_set_drive_state_leds,
  [DRIVE_FSM_OUTPUT_EVENT_PARKING] = prv_set_drive_state_leds,
};

StatusCode led_manager_init(void) {
  status_ok_or_return(mcp23008_gpio_init(MCP23008_I2C_PORT, MCP23008_I2C_ADDR));
  Mcp23008GpioSettings pin_settings = {
    .direction = MCP23008_GPIO_DIR_OUT,
    .state = MCP23008_GPIO_STATE_LOW,
  };
  for (CentreConsoleLed led = 0; led < NUM_CENTRE_CONSOLE_LEDS; led++) {
    status_ok_or_return(mcp23008_gpio_init_pin(&s_led_to_address[led], &pin_settings));
  }
  return STATUS_CODE_OK;
}

bool led_manager_process_event(Event *e) {
  if (e == NULL || e->id >= NUM_CENTRE_CONSOLE_EVENTS) return false;
  EventHandler handler = s_event_to_handler[e->id];
  if (handler != NULL) {
    handler(e);
    return true;
  }
  return false;
}
