#include "current_measurement_config.h"
#include "pin_defs.h"

// Definitions of the hardware configs declared in the header

#define POWER_DISTRIBUTION_I2C_PORT I2C_PORT_2

const PowerDistributionCurrentHardwareConfig FRONT_POWER_DISTRIBUTION_CURRENT_HW_CONFIG = {
  .bts7040s = (PowerDistributionBts7040Data[]){ {
                                                    .mux_selection = 13,
                                                },
                                                {
                                                    .mux_selection = 8,
                                                },
                                                {
                                                    .mux_selection = 10,
                                                },
                                                {
                                                    .mux_selection = 11,
                                                },
                                                {
                                                    .mux_selection = 3,
                                                },
                                                {
                                                    .mux_selection = 5,
                                                } },
  .num_bts7040_channels = 6,
  .mux_address =
      {
          .bit_width = 4,
          .sel_pins =
              {
                  { .port = GPIO_PORT_A, .pin = 6 },  //
                  { .port = GPIO_PORT_A, .pin = 5 },  //
                  { .port = GPIO_PORT_A, .pin = 4 },  //
                  { .port = GPIO_PORT_A, .pin = 3 },  //
              },
          .mux_output_pin = { .port = GPIO_PORT_A, .pin = 7 },  //
          .mux_enable_pin = { .port = GPIO_PORT_A, .pin = 2 },  //
      },
};

// This is based on https://uwmidsun.atlassian.net/wiki/x/GgODP, assuming that the currents in
// each row are connected to the same BTS7200s/BTS7040s (on the same pins with same mux selections).
const PowerDistributionCurrentHardwareConfig REAR_POWER_DISTRIBUTION_CURRENT_HW_CONFIG = {
  .bts7040s =
      (PowerDistributionBts7040Data[]){
          {
              .mux_selection = 8,
          },
          {
              .mux_selection = 13,
          },
          {
              .mux_selection = 11,
          },
          {
              .mux_selection = 10,
          },
          {
              .mux_selection = 3,
          },
          {
              .mux_selection = 5,
          },
      },
  .num_bts7040_channels = 6,
  .mux_address =
      {
          .bit_width = 4,
          .sel_pins =
              {
                  { .port = GPIO_PORT_A, .pin = 6 },  //
                  { .port = GPIO_PORT_A, .pin = 5 },  //
                  { .port = GPIO_PORT_A, .pin = 4 },  //
                  { .port = GPIO_PORT_A, .pin = 3 },  //
              },
          .mux_output_pin = { .port = GPIO_PORT_A, .pin = 7 },  //
          .mux_enable_pin = { .port = GPIO_PORT_A, .pin = 2 },  //
      },
};
