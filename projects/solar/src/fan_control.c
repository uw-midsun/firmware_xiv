#include "fan_control.h"

#include <stdbool.h>
#include "fault_monitor.h"
#include "gpio.h"
#include "gpio_it.h"
#include "solar_boards.h"
#include "solar_config.h"
#include "solar_events.h"
#include "spv1020_mppt.h"

/*
GpioSettings FanControlSettings = {

  .direction = GPIO_DIR_IN,
  .state = GPIO_STATE_LOW,
  .resistor = GPIO_RES_NONE,
  .alt_function = GPIO_ALTFN_ANALOG,
};

  (&pins).overtemp_addr = { .port = GPIO_PORT_B, .pin = 5 };
  (&pins).fan_fail_addr = { .port = GPIO_PORT_B, .pin = 7 };
  (&pins).full_speed_addr = { .port = GPIO_PORT_B, .pin = 6 };

*/

static FanControlSettings s_settings;

StatusCode fan_control_init(FanControlSettings *settings) {
  if (settings == NULL || settings->mppt_count > MAX_SOLAR_BOARD_MPPTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  GpioSettings fan_pin_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  GpioSettings full_speed_pin_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&settings->overtemp_addr, &fan_pin_settings);
  gpio_init_pin(&settings->fan_fail_addr, &fan_pin_settings);
  gpio_init_pin(&settings->full_speed_addr, &full_speed_pin_settings);

  s_settings = *settings;
  return status_code(STATUS_CODE_OK);
}

static void prv_fanfail_callback(void) {
  LOG_WARN("fan_control detected fanfail, raising fault event");
  fault_handler_raise_fault(EE_SOLAR_FAULT_FAN_FAIL, 0);
}

static void prv_overtemp_callback(void) {
  LOG_WARN("fan_control detected overtemperature, raising fault event");
  fault_handler_raise_fault(EE_SOLAR_FAULT_FAN_OVERTEMPERATURE, 0);
}

static void prv_check_temperature(uint8_t thermistor) {
  bool is_set = false;
  data_store_get_is_set(DATA_POINT_TEMPERATURE(thermistor), &is_set);
  if (is_set) {
    uint32_t value = 0;
    data_store_get(DATA_POINT_TEMPERATURE(thermistor), &value);
    if (value >= s_settings.full_speed_temp_threshold) {
      gpio_set_state(&s_settings.full_speed_addr, GPIO_STATE_LOW);
      fault_handler_raise_fault(EE_SOLAR_FAULT_FAN_OVERTEMPERATURE, thermistor);
    }
  }
}

bool fan_control_process_event(Event *e) {
  if (e != NULL && e->id == DATA_READY_EVENT) {
    for (Mppt mppt = 0; mppt < s_settings.mppt_count; mppt++) {
      prv_check_temperature(mppt);
      if (spv1020_is_overtemperature(DATA_POINT_MPPT_STATUS(mppt))) {
        gpio_set_state(&s_settings.full_speed_addr, GPIO_STATE_LOW);
        return true;
      }
    }
    gpio_set_state(&s_settings.full_speed_addr, GPIO_STATE_HIGH);
    return true;
  }
  return false;
}
