#include "fan_control_solar.h"

#include <stdbool.h>
#include "data_store.h"
#include "exported_enums.h"
#include "fault_handler.h"
#include "fault_monitor.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "max6643_fan_controller.h"
#include "solar_boards.h"
#include "solar_events.h"
#include "spv1020_mppt.h"

/*
GpioSettings FanControlSettingsSolar = {

  .direction = GPIO_DIR_IN,
  .state = GPIO_STATE_LOW,
  .resistor = GPIO_RES_NONE,
  .alt_function = GPIO_ALTFN_ANALOG,
};

  (&pins).overtemp_addr = { .port = GPIO_PORT_B, .pin = 5 };
  (&pins).fan_fail_addr = { .port = GPIO_PORT_B, .pin = 7 };
  (&pins).full_speed_addr = { .port = GPIO_PORT_B, .pin = 6 };

*/

static FanControlSettingsSolar s_settings;

static void prv_fanfail_callback(const GpioAddress *address, void *context) {
  LOG_WARN("fan_control detected fanfail, raising fault event");
  fault_handler_raise_fault(EE_SOLAR_FAULT_FAN_FAIL, 0);
}

static void prv_overtemp_callback(const GpioAddress *address, void *context) {
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
    }
  }
}

StatusCode fan_control_init(FanControlSettingsSolar *settings) {
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

  Max6643Settings max6643_init_settings = {
    .fanfail_pin = settings->fan_fail_addr,
    .overtemp_pin = settings->overtemp_addr,
    .fanfail_callback = &prv_fanfail_callback,
    .overtemp_callback = &prv_overtemp_callback,
    .fanfail_callback_context = NULL,
    .overtemp_callback_context = NULL,
  };
  max6643_init(&max6643_init_settings);

  return status_code(STATUS_CODE_OK);
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
    gpio_set_state(&s_settings.full_speed_addr, GPIO_STATE_HIGH); // raised twice
    return true;
  }
  return false;
}
