#include "../../../libraries/ms-common/inc/adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

typedef enum { B2_BUTTON = 0, NUM_BUTTONS } Button;

typedef enum { A6_SENSOR = 0, NUM_SENSORS } Sensor;

static GpioAddress button_addresses[] = {
  [B2_BUTTON] = { .port = GPIO_PORT_B, .pin = 2 },
};

static GpioAddress sensor_addresses[] = { [A6_SENSOR] = { .port = GPIO_PORT_A, .pin = 6 } };

static GpioSettings s_button_settings = {
  .direction = GPIO_DIR_IN, .alt_function = GPIO_ALTFN_NONE, .resistor = GPIO_RES_PULLDOWN
};

static GpioSettings sensor_settings = {
  .direction = GPIO_DIR_IN,           //
  .state = GPIO_STATE_LOW,            //
  .alt_function = GPIO_ALTFN_ANALOG,  //
  .resistor = GPIO_RES_NONE           //
};

static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,       //
  .priority = INTERRUPT_PRIORITY_NORMAL,  //
};

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  uint16_t sensor_data = 0;
  AdcChannel sensor_channel = (AdcChannel)context;
  adc_read_raw(sensor_channel, &sensor_data);
  LOG_DEBUG("Sensor Reading: %d\n", sensor_data);
}

static void prv_init_gpio_pins(void) {
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    gpio_init_pin(&button_addresses[i], &s_button_settings);
    gpio_init_pin(&sensor_addresses[i], &sensor_settings);
  }
}

static void prv_register_interrupts(void) {
  gpio_it_register_interrupt(&button_addresses[B2_BUTTON], &s_interrupt_settings,
                             INTERRUPT_EDGE_RISING, prv_button_interrupt_handler,
                             &sensor_addresses[A6_SENSOR]);
}

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void soft_counter_callback(const SoftTimerId timer_id, void *context) {
  Counters *counter1 = (Counters *)context;
  // LOG_DEBUG("counter1 address: %p \n", &counter1);
  counter1->counter_a = counter1->counter_a + 1;
  LOG_DEBUG("counter a: %i \n", counter1->counter_a);
  if (counter1->counter_a % 2 == 0) {
    counter1->counter_b = counter1->counter_b + 1;
    LOG_DEBUG("counter b: %i \n", counter1->counter_b);
  }
  soft_timer_start_millis(500, soft_counter_callback, counter1, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  prv_init_gpio_pins();

  adc_init(ADC_MODE_SINGLE);

  AdcChannel sensor_channel = NUM_ADC_CHANNELS;
  adc_get_channel(sensor_addresses[A6_SENSOR], &sensor_channel);
  adc_set_channel(sensor_channel, true);

  LOG_DEBUG("Hello world!\n");

  prv_register_interrupts();

  Counters counter1 = { .counter_b = 0, .counter_a = 0 };
  // LOG_DEBUG("counter1 address: %p \n", &counter1);
  soft_timer_start_millis(500, soft_counter_callback, &counter1, NULL);

  while (true) {
  }
  return 0;
}
