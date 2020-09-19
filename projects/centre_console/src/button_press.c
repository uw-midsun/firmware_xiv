#include "button_press.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "status.h"

static GpioAddress s_button_addresses[NUM_CENTRE_CONSOLE_BUTTONS] = {
  [CENTRE_CONSOLE_BUTTON_DRIVE] = { .port = GPIO_PORT_A, .pin = 5 },
  [CENTRE_CONSOLE_BUTTON_REVERSE] = { .port = GPIO_PORT_A, .pin = 7 },
  [CENTRE_CONSOLE_BUTTON_POWER] = { .port = GPIO_PORT_B, .pin = 0 },
  [CENTRE_CONSOLE_BUTTON_NEUTRAL] = { .port = GPIO_PORT_A, .pin = 6 },
  [CENTRE_CONSOLE_BUTTON_HAZARD] = { .port = GPIO_PORT_A, .pin = 1 },
  // TODO(SOFT-296): Remove ebrake
  [CENTRE_CONSOLE_BUTTON_PARKING] = { .port = GPIO_PORT_A, .pin = 0 },
};

static CentreConsoleButtonPressEvent s_button_event_lookup[NUM_CENTRE_CONSOLE_BUTTONS] = {
  [CENTRE_CONSOLE_BUTTON_DRIVE] = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_DRIVE,
  [CENTRE_CONSOLE_BUTTON_REVERSE] = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_REVERSE,
  [CENTRE_CONSOLE_BUTTON_POWER] = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_POWER,
  [CENTRE_CONSOLE_BUTTON_NEUTRAL] = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_NEUTRAL,
  [CENTRE_CONSOLE_BUTTON_PARKING] = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_PARKING,
  [CENTRE_CONSOLE_BUTTON_HAZARD] = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_HAZARD,
};

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  CentreConsoleButtonPressEvent *event_id = (CentreConsoleButtonPressEvent *)context;
  event_raise(*event_id, 0);
}

StatusCode button_press_init(void) {
  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };
  for (CentreConsoleButton button = 0; button < NUM_CENTRE_CONSOLE_BUTTONS; button++) {
    gpio_it_register_interrupt(&s_button_addresses[button], &interrupt_settings,
                               INTERRUPT_EDGE_RISING, prv_button_interrupt_handler,
                               &s_button_event_lookup[button]);
  }
  return STATUS_CODE_OK;
}

GpioAddress *test_provide_button_addresses(void) {
  return s_button_addresses;
}
