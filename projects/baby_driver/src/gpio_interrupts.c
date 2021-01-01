#include "gpio_interrupts.h"

#include <string.h>

#include "babydriver_msg_defs.h"
#include "can_transmit.h"
#include "dispatcher.h"
#include "gpio.h"
#include "gpio_it.h"

// Used to indicate that an interrupt has not been registered in interrupt_data
#define INTERRUPT_NOT_REGISTERED (NUM_INTERRUPT_EDGES + 1)

// Used to indicate that an interrupt is disabled in interrupt_data
#define INTERRUPT_DISABLED NUM_INTERRUPT_EDGES

static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL,
};

static GpioSettings s_gpio_settings = { .state = GPIO_STATE_LOW,
                                        .direction = GPIO_DIR_IN,
                                        .resistor = GPIO_RES_NONE,
                                        .alt_function = GPIO_ALTFN_NONE };

// For every GPIO pin the array stores whether it's interrupt is enabled
// When the value is INTERRUPT_NOT_REGISTERED the interrupt must be registered before being enabled
// When the value is INTERRUPT_DSIABLED the interrupt is disabled
// Other values indicate the interrupt edge type and that the interrupts have been enabled
static uint8_t s_interrupt_data[NUM_GPIO_PORTS][GPIO_PINS_PER_PORT];

static void prv_init_interrupt_data(void) {
  // All gpio pins begin in a disabled state where the interrupt must first be registered to be used
  for (uint8_t i = 0; i < NUM_GPIO_PORTS; i++) {
    for (uint8_t j = 0; j < GPIO_PINS_PER_PORT; j++) {
      s_interrupt_data[i][j] = INTERRUPT_NOT_REGISTERED;
    }
  }
}

static void prv_gpio_interrupt_handler(const GpioAddress *address, void *context) {
  GpioPort port = address->port;
  uint8_t pin = address->pin;
  InterruptEdge edge;

  // If the gpio state is high the edge is rising and if low the edge is falling
  GpioState input_state;
  gpio_get_state(address, &input_state);
  edge = (input_state == GPIO_STATE_HIGH) ? INTERRUPT_EDGE_RISING : INTERRUPT_EDGE_FALLING;

  // Interrupt is only triggered if the interrupt data indicates that the edge is
  // rising & falling, rising with a rising edge, or falling with a falling edge
  if (s_interrupt_data[port][pin] == INTERRUPT_EDGE_RISING_FALLING ||
      s_interrupt_data[port][pin] == edge || s_interrupt_data[port][pin] == edge) {
    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_GPIO_IT_INTERRUPT, port, pin, edge, 0, 0, 0, 0);
  }
}

static StatusCode prv_register_gpio_interrupt(uint8_t data[8], void *context, bool *tx_result) {
  GpioPort port = data[1];
  uint8_t pin = data[2];
  InterruptEdge edge = data[3];
  GpioAddress gpio_address = { .port = port, .pin = pin };

  // Checks for invalid port, pin, or interrupt edge
  if (port >= NUM_GPIO_PORTS || pin >= GPIO_PINS_PER_PORT || edge >= NUM_INTERRUPT_EDGES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // If the interrupt for a pin has yet to be registered it must be registered, otherwise
  // only the edge is recorded to prevent nonzero status code from using gpio_it_register_interrupt
  // on the same pin multiple times
  if (s_interrupt_data[port][pin] == INTERRUPT_NOT_REGISTERED) {
    status_ok_or_return(gpio_init_pin(&gpio_address, &s_gpio_settings));
    status_ok_or_return(gpio_it_register_interrupt(&gpio_address, &s_interrupt_settings,
                                                   INTERRUPT_EDGE_RISING_FALLING,
                                                   prv_gpio_interrupt_handler, NULL));
  }
  s_interrupt_data[port][pin] = edge;

  return STATUS_CODE_OK;
}

static StatusCode prv_unregister_gpio_interrupt(uint8_t data[8], void *context, bool *tx_result) {
  uint8_t port = data[1];
  uint8_t pin = data[2];

  // Checks for invalid port and pin
  if (port >= NUM_GPIO_PORTS || pin >= GPIO_PINS_PER_PORT) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // Invalid to try to unregister an interrupt that has not been registered
  if (s_interrupt_data[port][pin] == INTERRUPT_NOT_REGISTERED) {
    return STATUS_CODE_INVALID_ARGS;
  } else {
    s_interrupt_data[port][pin] = INTERRUPT_DISABLED;
  }

  return STATUS_CODE_OK;
}

StatusCode gpio_interrupts_init(void) {
  prv_init_interrupt_data();
  status_ok_or_return(dispatcher_register_callback(BABYDRIVER_MESSAGE_GPIO_IT_REGISTER_COMMAND,
                                                   prv_register_gpio_interrupt, NULL));
  return dispatcher_register_callback(BABYDRIVER_MESSAGE_GPIO_IT_UNREGISTER_COMMAND,
                                      prv_unregister_gpio_interrupt, NULL);
}
