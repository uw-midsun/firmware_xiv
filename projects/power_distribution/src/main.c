// Uncomment this line to force firmware to run as front or rear power distribution.
// #define FORCE_IS_FRONT_POWER_DISTRIBUTION true

#include <stdbool.h>
#include <stdint.h>

#include "adc.h"
#include "bps_watcher.h"
#include "bug.h"
#include "can_msg_defs.h"
#include "can_rx_event_mapper.h"
#include "can_rx_event_mapper_config.h"
#include "can_transmit.h"
#include "current_measurement.h"
#include "current_measurement_config.h"
#include "front_uv_detector.h"
#include "interrupt.h"
#include "lights_signal_fsm.h"
#include "log.h"
#include "output.h"
#include "output_config.h"
#include "pca9539r_gpio_expander.h"
#include "pd_error_defs.h"
#include "pd_events.h"
#include "pd_fan_ctrl.h"
#include "pd_gpio.h"
#include "pd_gpio_config.h"
#include "pin_defs.h"
#include "publish_data.h"
#include "publish_data_config.h"
#include "rear_strobe_blinker.h"
#include "smoketests_pd.h"
#include "voltage_regulator.h"
#include "wait.h"

#ifndef PD_SMOKE_TEST

#define CURRENT_MEASUREMENT_INTERVAL_US 1500000  // 1.5s between current measurements
#define SIGNAL_BLINK_INTERVAL_US 500000          // 0.5s between blinks of the signal lights
#define STROBE_BLINK_INTERVAL_US 100000          // 0.1s between blinks of the strobe light
#define NUM_SIGNAL_BLINKS_BETWEEN_SYNCS 10
#define VOLTAGE_REGULATOR_DELAY_MS 25

static CanStorage s_can_storage;
static SignalFsmStorage s_lights_signal_fsm_storage;

static bool prv_determine_is_front_pd(void) {
#ifdef FORCE_IS_FRONT_POWER_DISTRIBUTION
  return FORCE_IS_FRONT_POWER_DISTRIBUTION;
#else
  // initialize PA8
  GpioAddress board_test_pin = FRONT_OR_REAR_RECOGNITION_PIN;
  GpioSettings board_test_settings = {
    .direction = GPIO_DIR_IN,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  gpio_init_pin(&board_test_pin, &board_test_settings);

  // we're on front if it's low and rear if it's high
  GpioState state = GPIO_STATE_LOW;
  gpio_get_state(&board_test_pin, &state);

  return state == GPIO_STATE_LOW;
#endif
}

static StatusCode prv_init_i2c(void) {
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = PD_I2C_SCL_PIN,
    .sda = PD_I2C_SDA_PIN,
  };
  return i2c_init(PD_I2C_PORT, &i2c_settings);
}

static StatusCode prv_init_can(bool is_front_pd) {
  CanSettings can_settings = {
    .device_id = is_front_pd ? SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT
                             : SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR,
    .loopback = false,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = PD_CAN_EVENT_RX,
    .tx_event = PD_CAN_EVENT_TX,
    .fault_event = PD_CAN_EVENT_FAULT,
    .rx = PD_CAN_RX_PIN,
    .tx = PD_CAN_TX_PIN,
  };
  return can_init(&s_can_storage, &can_settings);
}

static void prv_voltage_monitor_error_callback(VoltageRegulatorError error, void *context) {
  uint16_t pd_err_flags = (error == VOLTAGE_REGULATOR_ERROR_OFF_WHEN_SHOULD_BE_ON)
                              ? (PD_5V_REG_ERROR | PD_5V_REG_DATA)
                              : PD_5V_REG_ERROR;
  bool is_front_pd = *(bool *)context;

  if (is_front_pd) {
    CAN_TRANSMIT_FRONT_PD_FAULT(pd_err_flags, 0);
  } else {
    CAN_TRANSMIT_REAR_PD_FAULT(pd_err_flags, 0, 0, 0);
  }
}

static void prv_current_measurement_data_ready_callback(void *context) {
  // called when current_measurement has new data: send it to publish_data for publishing
  CurrentMeasurementStorage *storage = current_measurement_get_storage();
  publish_data_publish(storage->measurements);
}

#endif  // PD_SMOKE_TEST

int main(void) {
#ifdef PD_SMOKE_TEST
  RUN_SMOKE_TEST(PD_SMOKE_TEST);
#else
  LOG_DEBUG("Initializing power distribution...\n");

  // initialize all libraries
  interrupt_init();
  soft_timer_init();
  BUG(gpio_init());
  gpio_it_init();
  event_queue_init();
  adc_init(ADC_MODE_SINGLE);
  BUG(prv_init_i2c());

  // test if it's front or rear PD, and log for easier debugging if initialization fails
  bool is_front_pd = prv_determine_is_front_pd();
  LOG_DEBUG("Detected %s power distribution board...\n", is_front_pd ? "front" : "rear");

  BUG(prv_init_can(is_front_pd));

  // initialize bps watcher, output, can_rx_event_mapper, gpio, publish_data
  BUG(bps_watcher_init());
  BUG(output_init(&g_combined_output_config, is_front_pd));
  BUG(can_rx_event_mapper_init(is_front_pd ? &g_front_can_rx_config : &g_rear_can_rx_config));
  BUG(pd_gpio_init(is_front_pd ? &g_front_pd_gpio_config : &g_rear_pd_gpio_config));
  BUG(publish_data_init(is_front_pd ? &g_front_publish_data_config : &g_rear_publish_data_config));

  // Initialize Voltage Regulator
  VoltageRegulatorSettings vreg_set = {
    .enable_pin = PD_5V_REG_ENABLE,
    .monitor_pin = PD_5V_REG_MONITOR_PIN,
    .timer_callback_delay_ms = VOLTAGE_REGULATOR_DELAY_MS,
    .error_callback = prv_voltage_monitor_error_callback,
    .error_callback_context = (const void *)(&is_front_pd),
  };
  VoltageRegulatorStorage vreg_store = { 0 };
  BUG(voltage_regulator_init(&vreg_store, &vreg_set));
  BUG(voltage_regulator_set_enabled(&vreg_store, true));

  // initialize current_measurement
  CurrentMeasurementSettings current_measurement_settings = {
    .config =
        is_front_pd ? &g_front_current_measurement_config : &g_rear_current_measurement_config,
    .interval_us = CURRENT_MEASUREMENT_INTERVAL_US,
    .callback = &prv_current_measurement_data_ready_callback,
  };
  BUG(current_measurement_init(&current_measurement_settings));

  // initialize lights_signal_fsm
  SignalFsmSettings lights_signal_fsm_settings = {
    .signal_left_input_event = PD_SIGNAL_EVENT_LEFT,
    .signal_right_input_event = PD_SIGNAL_EVENT_RIGHT,
    .signal_hazard_input_event = PD_SIGNAL_EVENT_HAZARD,
    .signal_left_output_event = PD_GPIO_EVENT_SIGNAL_LEFT,
    .signal_right_output_event = PD_GPIO_EVENT_SIGNAL_RIGHT,
    .signal_hazard_output_event = PD_GPIO_EVENT_SIGNAL_HAZARD,
    .event_priority = PD_ACTION_EVENT_PRIORITY,
    .blink_interval_us = SIGNAL_BLINK_INTERVAL_US,
    .sync_behaviour = is_front_pd ? LIGHTS_SYNC_BEHAVIOUR_RECEIVE_SYNC_MSGS
                                  : LIGHTS_SYNC_BEHAVIOUR_SEND_SYNC_MSGS,
    .sync_event = PD_SYNC_EVENT_LIGHTS,
    .num_blinks_between_syncs = NUM_SIGNAL_BLINKS_BETWEEN_SYNCS,
  };
  BUG(lights_signal_fsm_init(&s_lights_signal_fsm_storage, &lights_signal_fsm_settings));

  // initialize fan ctrl
  FanCtrlSettings fan_settings = {
    .i2c_port = PD_I2C_PORT,
    .i2c_address = ADT7476A_I2C_ADDRESS,
  };
  if (is_front_pd) {
    fan_settings.fan_pwm1 = FRONT_PD_PWM_1;
    fan_settings.fan_pwm2 = FRONT_PD_PWM_2;
  } else {
    fan_settings.fan_pwm1 = REAR_ENC_VENT_PWM;
    fan_settings.fan_pwm2 = REAR_DCDC_PWM;
  }
  StatusCode fan_ctrl_status = pd_fan_ctrl_init(&fan_settings, is_front_pd);
  if (!status_ok(fan_ctrl_status)) {
    // Fan control could in theory fail to init spontaneously (due to I2C issues with ADT7476A), and
    // we can survive without it, so we don't use BUG and instead just log.
    LOG_WARN("Failed to initialize fan control! Status=%d\n", fan_ctrl_status);
  }

  if (is_front_pd) {
    // initialize UV cutoff detector on front
    BUG(front_uv_detector_init(&(GpioAddress)FRONT_UV_COMPARATOR_PIN));
  } else {
    // initialize strobe_blinker on rear
    RearStrobeBlinkerSettings strobe_blinker_settings = {
      .strobe_blink_delay_us = STROBE_BLINK_INTERVAL_US,
    };
    BUG(rear_strobe_blinker_init(&strobe_blinker_settings));
  }

  LOG_DEBUG("Power distribution successfully initialized as %s.\n", is_front_pd ? "front" : "rear");

  // process events
  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
      pd_gpio_process_event(&e);
      lights_signal_fsm_process_event(&s_lights_signal_fsm_storage, &e);
      if (!is_front_pd) {
        rear_strobe_blinker_process_event(&e);
      }
    }
    wait();
  }
#endif  // PD_SMOKE_TEST

  return 0;
}
