// If pins 30 and 31 haven't been grounded on rear power distribution yet, uncomment this line
// and it'll force firmware to run as rear power distribution.
// #define FORCE_REAR_POWER_DISTRIBUTION

#include "adc.h"
#include "can_msg_defs.h"
#include "interrupt.h"
#include "lights_signal_fsm.h"
#include "log.h"
#include "pca9539r_gpio_expander.h"
#include "power_distribution_can_rx_event_mapper.h"
#include "power_distribution_can_rx_event_mapper_config.h"
#include "power_distribution_current_measurement.h"
#include "power_distribution_current_measurement_config.h"
#include "power_distribution_events.h"
#include "power_distribution_gpio.h"
#include "power_distribution_gpio_config.h"
#include "power_distribution_pin_defs.h"
#include "power_distribution_publish_data.h"
#include "power_distribution_publish_data_config.h"
#include "rear_power_distribution_strobe_blinker.h"

#define CURRENT_MEASUREMENT_INTERVAL_US 500000  // 0.5s between current measurements
#define SIGNAL_BLINK_INTERVAL_US 500000         // 0.5s between blinks of the signal lights
#define STROBE_BLINK_INTERVAL_US 100000         // 0.1s between blinks of the strobe light
#define NUM_SIGNAL_BLINKS_BETWEEN_SYNCS 10

static CanStorage s_can_storage;
static SignalFsmStorage s_lights_signal_fsm_storage;

static bool prv_determine_is_front_power_distribution(void) {
#ifdef FORCE_REAR_POWER_DISTRIBUTION
  return false;
#else
  // initialize pin 30 (PC13) as pull-up, it's shorted on rear
  GpioAddress board_test_pin = { .port = GPIO_PORT_C, .pin = 13 };
  GpioSettings board_test_settings = {
    .direction = GPIO_DIR_IN,
    .resistor = GPIO_RES_PULLUP,
    .alt_function = GPIO_ALTFN_NONE,
  };
  gpio_init_pin(&board_test_pin, &board_test_settings);

  // we're on front if it's high and rear if it's low
  GpioState state = GPIO_STATE_HIGH;  // default to front since we can force rear
  gpio_get_state(&board_test_pin, &state);

  return state == GPIO_STATE_HIGH;
#endif
}

static void prv_init_i2c(void) {
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = POWER_DISTRIBUTION_I2C_SCL_PIN,
    .sda = POWER_DISTRIBUTION_I2C_SDA_PIN,
  };
  i2c_init(POWER_DISTRIBUTION_I2C_PORT, &i2c_settings);
}

static void prv_init_can(bool is_front_power_distribution) {
  CanSettings can_settings = {
    .device_id = is_front_power_distribution ? SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT
                                             : SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR,
    .loopback = false,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = POWER_DISTRIBUTION_CAN_EVENT_RX,
    .tx_event = POWER_DISTRIBUTION_CAN_EVENT_TX,
    .fault_event = POWER_DISTRIBUTION_CAN_EVENT_FAULT,
    .rx = POWER_DISTRIBUTION_CAN_RX_PIN,
    .tx = POWER_DISTRIBUTION_CAN_TX_PIN,
  };
  can_init(&s_can_storage, &can_settings);
}

static void prv_current_measurement_data_ready_callback(void *context) {
  // called when current_measurement has new data: send it to publish_data for publishing
  PowerDistributionCurrentStorage *storage = power_distribution_current_measurement_get_storage();
  power_distribution_publish_data_publish(storage->measurements);
}

int main(void) {
  // initialize all libraries
  interrupt_init();
  soft_timer_init();
  gpio_init();
  event_queue_init();
  adc_init(ADC_MODE_SINGLE);
  prv_init_i2c();

  pca9539r_gpio_init(POWER_DISTRIBUTION_I2C_PORT, POWER_DISTRIBUTION_I2C_ADDRESS_0);
  pca9539r_gpio_init(POWER_DISTRIBUTION_I2C_PORT, POWER_DISTRIBUTION_I2C_ADDRESS_1);

  // test if it's front or rear power distribution
  bool is_front_power_distribution = prv_determine_is_front_power_distribution();

  prv_init_can(is_front_power_distribution);

  // initialize can_rx_event_mapper, gpio, publish_data
  power_distribution_can_rx_event_mapper_init(is_front_power_distribution
                                                  ? FRONT_POWER_DISTRIBUTION_CAN_RX_CONFIG
                                                  : REAR_POWER_DISTRIBUTION_CAN_RX_CONFIG);
  power_distribution_gpio_init(is_front_power_distribution ? FRONT_POWER_DISTRIBUTION_GPIO_CONFIG
                                                           : REAR_POWER_DISTRIBUTION_GPIO_CONFIG);
  power_distribution_publish_data_init(is_front_power_distribution
                                           ? FRONT_POWER_DISTRIBUTION_PUBLISH_DATA_CONFIG
                                           : REAR_POWER_DISTRIBUTION_PUBLISH_DATA_CONFIG);

  // initialize current_measurement
  PowerDistributionCurrentSettings current_measurement_settings = {
    .hw_config = is_front_power_distribution ? FRONT_POWER_DISTRIBUTION_CURRENT_HW_CONFIG
                                             : REAR_POWER_DISTRIBUTION_CURRENT_HW_CONFIG,
    .interval_us = CURRENT_MEASUREMENT_INTERVAL_US,
    .callback = &prv_current_measurement_data_ready_callback,
  };
  power_distribution_current_measurement_init(&current_measurement_settings);

  // initialize lights_signal_fsm
  SignalFsmSettings lights_signal_fsm_settings = {
    .signal_left_input_event = POWER_DISTRIBUTION_SIGNAL_EVENT_LEFT,
    .signal_right_input_event = POWER_DISTRIBUTION_SIGNAL_EVENT_RIGHT,
    .signal_hazard_input_event = POWER_DISTRIBUTION_SIGNAL_EVENT_HAZARD,
    .signal_left_output_event = POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT,
    .signal_right_output_event = POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_RIGHT,
    .signal_hazard_output_event = POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD,
    .blink_interval_us = SIGNAL_BLINK_INTERVAL_US,
    .sync_behaviour = is_front_power_distribution ? LIGHTS_SYNC_BEHAVIOUR_RECEIVE_SYNC_MSGS
                                                  : LIGHTS_SYNC_BEHAVIOUR_SEND_SYNC_MSGS,
    .sync_event = POWER_DISTRIBUTION_SYNC_EVENT_LIGHTS,
    .num_blinks_between_syncs = NUM_SIGNAL_BLINKS_BETWEEN_SYNCS,
  };
  lights_signal_fsm_init(&s_lights_signal_fsm_storage, &lights_signal_fsm_settings);

  // initialize strobe_blinker on rear
  if (!is_front_power_distribution) {
    RearPowerDistributionStrobeBlinkerSettings strobe_blinker_settings = {
      .strobe_blink_delay_us = STROBE_BLINK_INTERVAL_US,
    };
    rear_power_distribution_strobe_blinker_init(&strobe_blinker_settings);
  }

  LOG_DEBUG("Hello from power distribution, initialized as %s\r\n",
            is_front_power_distribution ? "front" : "rear");

  // process events
  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
      can_process_event(&e);
      power_distribution_gpio_process_event(&e);
      lights_signal_fsm_process_event(&s_lights_signal_fsm_storage, &e);
      if (!is_front_power_distribution) {
        rear_power_distribution_strobe_blinker_process_event(&e);
      }
    }
  }

  return 0;
}
