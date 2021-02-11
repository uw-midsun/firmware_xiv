#pragma once

#include <stdbool.h>
#include "fault_monitor.h"
#include "gpio.h"
#include "gpio_it.h"
#include "solar_events.h"

typedef struct FanControlSettingsSolar {
  GpioAddress overtemp_addr;
  GpioAddress fan_fail_addr;
  GpioAddress full_speed_addr;
  uint16_t full_speed_temp_threshold;

  SolarMpptCount mppt_count;
} FanControlSettingsSolar;

StatusCode fan_control_init(FanControlSettingsSolar *settings);

bool fan_control_process_event(Event *e);
