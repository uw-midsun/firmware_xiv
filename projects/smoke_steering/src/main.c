#include "adc.h"
#include "adc_periodic_reader.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define HORN_GPIO_ADDR \
  { .port = GPIO_PORT_B, .pin = 1 }
#define RADIO_PPT_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 6 }
#define HIGH_BEAM_FORWARD_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 7 }
#define HIGH_BEAM_REAR_GPIO_ADDR \
  { .port = GPIO_PORT_B, .pin = 0 }
#define REGEN_BRAKE_TOGGLE_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 4 }
#define CC_TOGGLE_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 5 }
#define CC_INCREASE_SPEED_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 2 }
#define CC_DECREASE_SPEED_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 1 }

#define TIMER_INTERVAL_MS 1000
#define VOLTAGE_TOLERANCE_MV 100
#define STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV 1000
#define STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV 2000
#define ADC_READER_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 3 }

typedef enum {
  STEERING_DIGITAL_INPUT_HORN = 0,
  STEERING_DIGITAL_INPUT_RADIO_PPT,
  STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD,
  STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR,
  STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE,
  STEERING_DIGITAL_INPUT_CC_TOGGLE,
  STEERING_DIGITAL_INPUT_CC_INCREASE_SPEED,
  STEERING_DIGITAL_INPUT_CC_DECREASE_SPEED,
  NUM_STEERING_DIGITAL_INPUTS,
} SteeringInterfaceDigitalInput;

static GpioAddress s_steering_address_lookup_table[NUM_STEERING_DIGITAL_INPUTS] = {
  [STEERING_DIGITAL_INPUT_HORN] = HORN_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_RADIO_PPT] = RADIO_PPT_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD] = HIGH_BEAM_FORWARD_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR] = HIGH_BEAM_REAR_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE] = REGEN_BRAKE_TOGGLE_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_CC_TOGGLE] = CC_TOGGLE_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_CC_INCREASE_SPEED] = CC_INCREASE_SPEED_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_CC_DECREASE_SPEED] = CC_INCREASE_SPEED_GPIO_ADDR,
};

static char *s_steering_input_lookup_table[NUM_STEERING_DIGITAL_INPUTS] = {
  [STEERING_DIGITAL_INPUT_HORN] = "Horn",
  [STEERING_DIGITAL_INPUT_RADIO_PPT] = "Radio",
  [STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD] = "High Beam Forward",
  [STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR] = "High Beam Rear",
  [STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE] = "Regenerative Braking",
  [STEERING_DIGITAL_INPUT_CC_TOGGLE] = "Cruise Control",
  [STEERING_DIGITAL_INPUT_CC_INCREASE_SPEED] = "Increase Speed Cruise Control",
  [STEERING_DIGITAL_INPUT_CC_DECREASE_SPEED] = "Decrease Speed Cruise Control",
};

void prv_callback_log_button(const GpioAddress *address, void *context) {
  char *input = (char *)context;
  printf("%s\n", input);
}

void prv_callback_log_adc(uint16_t data, PeriodicReaderId id, void *context) {
  if (data > STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV &&
      data < STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV) {
    printf("Left signal\n");
  } else if (data > STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV &&
             data < STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV) {
    printf("Right Signal\n");
  }
  printf("ADC Value: %d\n", data);
}

void prv_init_buttons() {
  GpioSettings digital_input_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };

  for (int i = 0; i < NUM_STEERING_DIGITAL_INPUTS; i++) {
    gpio_init_pin(&s_steering_address_lookup_table[i], &digital_input_settings);

    InterruptSettings interrupt_settings = { .type = INTERRUPT_TYPE_INTERRUPT,
                                             .priority = INTERRUPT_PRIORITY_NORMAL };

    if (i == STEERING_DIGITAL_INPUT_HORN || i == STEERING_DIGITAL_INPUT_RADIO_PPT ||
        i == STEERING_DIGITAL_INPUT_CC_INCREASE_SPEED ||
        i == STEERING_DIGITAL_INPUT_CC_DECREASE_SPEED) {
      gpio_it_register_interrupt(&s_steering_address_lookup_table[i], &interrupt_settings,
                                 INTERRUPT_EDGE_RISING_FALLING, prv_callback_log_button,
                                 &s_steering_input_lookup_table[i]);
    } else {
      gpio_it_register_interrupt(&s_steering_address_lookup_table[i], &interrupt_settings,
                                 INTERRUPT_EDGE_FALLING, prv_callback_log_button,
                                 &s_steering_input_lookup_table[i]);
    }
  }
}

void prv_init_stalk() {
  AdcPeriodicReaderSettings reader_settings = {
    .address = ADC_READER_GPIO_ADDR,
    .callback = prv_callback_log_adc,
  };
  adc_periodic_reader_set_up_reader(PERIODIC_READER_ID_0, &reader_settings);
  adc_periodic_reader_start(PERIODIC_READER_ID_0);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();
  adc_init(ADC_MODE_SINGLE);
  adc_periodic_reader_init(TIMER_INTERVAL_MS);

  prv_init_buttons();
  prv_init_stalk();

Loop:
  goto Loop;

  return 0;
}
