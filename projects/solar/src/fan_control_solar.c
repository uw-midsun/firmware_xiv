// Contains the FanControlSolarSettings struc and declarations for functions to be defined here
#include "fan_control_solar.h"

#include <stdbool.h>

#include "data_store.h"
#include "exported_enums.h"
#include "fault_handler.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "max6643_fan_controller.h"
#include "solar_boards.h"
#include "solar_events.h"
#include "spv1020_mppt.h"

// Full Speed Pin is active low
#define FULL_SPEED_STATE_ENABLED GPIO_STATE_LOW
#define FULL_SPEED_STATE_DISABLED GPIO_STATE_HIGH

static FanControlSolarSettings s_settings;

static void prv_fanfail_callback(const GpioAddress *address, void *context) {
  LOG_WARN("fan_control_solar detected fanfail, raising fault event\n");
  fault_handler_raise_fault(EE_SOLAR_FAULT_FAN_FAIL, 0);
}

static void prv_overtemp_callback(const GpioAddress *address, void *context) {
  LOG_WARN("fan_control_solar detected overtemperature, raising fault event\n");
  fault_handler_raise_fault(EE_SOLAR_FAULT_FAN_OVERTEMPERATURE, 0);
}

// Checks the temperature for one mppt
// Will return true if full speed state should be enabled
static bool prv_check_temperature(uint8_t thermistor) {
  bool is_set = false;
  data_store_get_is_set(DATA_POINT_TEMPERATURE(thermistor), &is_set);
  if (is_set) {
    uint32_t value = 0;
    data_store_get(DATA_POINT_TEMPERATURE(thermistor), &value);
    return (value >= s_settings.full_speed_temp_threshold_dC);
  }
  return false;
}

// Checks the temperature for all mppts and enables mppt pin accordingly
// Will return true if full speed state should be enabled
static bool prv_are_mppts_overtemp(SolarMpptCount mppt_count) {
  for (Mppt mppt = 0; mppt < mppt_count; mppt++) {
    uint32_t status_value = 0;
    data_store_get(DATA_POINT_MPPT_STATUS(mppt), &status_value);
    if (prv_check_temperature(mppt)) {
      return true;
    } else if (spv1020_is_overtemperature(status_value)) {
      return true;
    }
  }
  return false;
}

StatusCode fan_control_init(FanControlSolarSettings *settings) {
  if (settings == NULL || settings->mppt_count > MAX_SOLAR_BOARD_MPPTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  GpioSettings fan_pin_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  GpioSettings full_speed_pin_settings = {
    .direction = GPIO_DIR_OUT,
    .state = FULL_SPEED_STATE_DISABLED,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  status_ok_or_return(gpio_init_pin(&settings->overtemp_addr, &fan_pin_settings));
  status_ok_or_return(gpio_init_pin(&settings->fan_fail_addr, &fan_pin_settings));
  status_ok_or_return(gpio_init_pin(&settings->full_speed_addr, &full_speed_pin_settings));

  s_settings = *settings;

  Max6643Settings max6643_init_settings = {
    .fanfail_pin = settings->fan_fail_addr,
    .overtemp_pin = settings->overtemp_addr,
    .fanfail_callback = prv_fanfail_callback,
    .overtemp_callback = prv_overtemp_callback,
    .fanfail_callback_context = NULL,
    .overtemp_callback_context = NULL,
  };
  status_ok_or_return(max6643_init(&max6643_init_settings));

  return status_code(STATUS_CODE_OK);
}

// Processes the next event returning true if the event is a DATA_READY_EVENT and is processed
// successfully. Also sets full speed pin high if an overtemp fault is detected.
// Returns false otherwise.
bool fan_control_process_event(Event *e) {
  if (e != NULL && e->id == DATA_READY_EVENT) {
    if (prv_are_mppts_overtemp(s_settings.mppt_count)) {
      gpio_set_state(&s_settings.full_speed_addr, FULL_SPEED_STATE_ENABLED);
      return true;
    }
    gpio_set_state(&s_settings.full_speed_addr, FULL_SPEED_STATE_DISABLED);
    return true;
  }
  return false;
}
