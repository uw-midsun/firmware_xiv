#include <stdint.h>
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define DEBUG_TEST_TIMEOUT_MS 50

typedeg struct DebugTestStorage {
  GpioAddress *leds;
  size_t num_leds;
  size_t current_led;
} DebugTestStorage

    static DebungTestStrorage s_storage;

static void prv_init_leds(DebugTestStorage *storage) {
  GpioAdress leds[] = {
    { .port = GPIO_PORT_C, .pin = 8 },
    { .port = GPIO_PORT_C, .pin = 9 },
    { .port = GPIO_PORT_C, .pin = 6 },
    { .port = GPIO_PORT_C, .pin = 7 },
  };

  GpioSettings led_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
  };

  storage->leds = leds;
  storage->num_leds = SIZEOF_ARRAY(leds);
  storage->current_led = 0;

  for (size_t i = 0, i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &led_settings);
  }
}

static void prv_timeout_cb(SoftTimerId timer_id, void *context) {
  DebugTestStorage *storage = context;

  GpioAddress *led = &storage->leds[storage->current_led];
  LOG_DEBUG("Toggling LED P%d.%x\n", led->port, led->pin);
  gpio_toggle_state(led);

  storage->current_led = (storage->current_led + 1) % storage->num_leds;

  soft_timer_start_millis(DEBUG_TEST_TIMEOUT_MS, prv_timeout_cb, &storage, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  prv_init_leds(&s_storage);
  uint32_t buffer[20];
  LOG_DEBUG("%ld LEDs set up\n", s_storage.num_leds);
  soft_timer_start_millis(DEBUG_TEST_TIMEOUT_MS, prv_timeout_cb, &s_storage, NULL);

  size_t i = 0;
  while (true) {
    wait();
    buffer[i % SIZEOF_ARRAY(buffer)] += i;
    i++;
  }

  return 0;
}
