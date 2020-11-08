#include "button_press.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "status.h"

static GpioAddress s_button_addresses[NUM_CENTRE_CONSOLE_BUTTONS] = {
  [CENTRE_CONSOLE_BUTTON_POWER] = { .port = GPIO_PORT_B, .pin = 0 },
  [CENTRE_CONSOLE_BUTTON_PARKING] = { .port = GPIO_PORT_A, .pin = 0 },
  [CENTRE_CONSOLE_BUTTON_HAZARDS] = { .port = GPIO_PORT_A, .pin = 1 },
  [CENTRE_CONSOLE_BUTTON_DRIVE] = { .port = GPIO_PORT_A, .pin = 5 },
  [CENTRE_CONSOLE_BUTTON_NEUTRAL] = { .port = GPIO_PORT_A, .pin = 6 },
  [CENTRE_CONSOLE_BUTTON_REVERSE] = { .port = GPIO_PORT_A, .pin = 7 },
};

static CentreConsoleButtonPressEvent s_button_event_lookup[NUM_CENTRE_CONSOLE_BUTTONS] = {
  [CENTRE_CONSOLE_BUTTON_POWER] = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_POWER,
  [CENTRE_CONSOLE_BUTTON_PARKING] = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_PARKING,
  [CENTRE_CONSOLE_BUTTON_HAZARDS] = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_HAZARD,
  [CENTRE_CONSOLE_BUTTON_DRIVE] = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_DRIVE,
  [CENTRE_CONSOLE_BUTTON_NEUTRAL] = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_NEUTRAL,
  [CENTRE_CONSOLE_BUTTON_REVERSE] = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_REVERSE,
};

static const char *s_button_names[NUM_CENTRE_CONSOLE_BUTTONS] = {
  [CENTRE_CONSOLE_BUTTON_POWER] = "power button",
  [CENTRE_CONSOLE_BUTTON_PARKING] = "parking button",
  [CENTRE_CONSOLE_BUTTON_HAZARDS] = "hazard button",
  [CENTRE_CONSOLE_BUTTON_DRIVE] = "drive button",
  [CENTRE_CONSOLE_BUTTON_NEUTRAL] = "neutral button",
  [CENTRE_CONSOLE_BUTTON_REVERSE] = "reverse button",
};

static bool s_button_is_latching[NUM_CENTRE_CONSOLE_BUTTONS] = {
  [CENTRE_CONSOLE_BUTTON_HAZARDS] = true,
};

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  CentreConsoleButtonPressEvent *event_id = (CentreConsoleButtonPressEvent *)context;
  GpioState button_state = NUM_GPIO_STATES;
  gpio_get_state(address, &button_state);
  event_raise(*event_id, button_state);
  for (uint8_t i = 0; i < NUM_CENTRE_CONSOLE_BUTTONS; i++) {
    if (s_button_event_lookup[i] == *event_id) {
      LOG_DEBUG("%s interrupt, raising event (state=%d)\n", s_button_names[i], button_state);
    }
  }
}

StatusCode button_press_init(void) {
  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };
  GpioSettings gpio_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE,
  };

  for (CentreConsoleButton button = 0; button < NUM_CENTRE_CONSOLE_BUTTONS; button++) {
    // initialize the pin as input so we can get its state later
    gpio_init_pin(&s_button_addresses[button], &gpio_settings);

    InterruptEdge edge =
        s_button_is_latching[button] ? INTERRUPT_EDGE_RISING_FALLING : INTERRUPT_EDGE_RISING;
    gpio_it_register_interrupt(&s_button_addresses[button], &interrupt_settings, edge,
                               prv_button_interrupt_handler, &s_button_event_lookup[button]);
  }

  return STATUS_CODE_OK;
}

GpioAddress *test_provide_button_addresses(void) {
  return s_button_addresses;
}
