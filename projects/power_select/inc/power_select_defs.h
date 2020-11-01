#pragma once

// Pin defintions + other info for power select

// Scaling factors
#define POWER_SELECT_PWR_SUP_ISENSE_SCALING 80       // mV/A
#define POWER_SELECT_PWR_SUP_VSENSE_SCALING 189.655  // mV/V
#define POWER_SELECT_DCDC_ISENSE_SCALING 80          // mV/A
#define POWER_SELECT_DCDC_VSENSE_SCALING 189.655     // mV/V
#define POWER_SELECT_AUX_ISENSE_SCALING 1            // mV/A
#define POWER_SELECT_AUX_VSENSE_SCALING 189.655      // mV/V

#define POWER_SELECT_AUX_VREF_MV 1624  // todo figure out if this is just for isense or

// Maximium measurements
#define POWER_SELECT_PWR_SUP_MAX_CURRENT_MA 37500
#define POWER_SELECT_PWR_SUP_MAX_VOLTAGE_MV 15820
#define POWER_SELECT_DCDC_MAX_CURRENT_MA 37500
#define POWER_SELECT_DCDC_MAX_VOLTAGE_MV 15820
#define POWER_SELECT_AUX_MAX_CURRENT_MA 8220
#define POWR_SELECT_AUX_MAX_VOLTAGE_MV 15820

// Sense pins
#define POWER_SELECT_PWR_SUP_ISENSE_ADDR \
  { GPIO_PORT_A, 7 }
#define POWER_SELECT_DCDC_ISENSE_ADDR \
  { GPIO_PORT_A, 6 }
#define POWER_SELECT_AUX_ISENSE_ADDR \
  { GPIO_PORT_A, 5 }
#define POWER_SELECT_DCDC_TEMP_ADDR \
  { GPIO_PORT_A, 4 }
#define POWER_SELECT_AUX_TEMP_ADDR \
  { GPIO_PORT_A, 3 }
#define POWER_SELECT_PWR_SUP_VSENSE_ADDR \
  { GPIO_PORT_A, 2 }
#define POWER_SELECT_DCDC_VSENSE_ADDR \
  { GPIO_PORT_A, 1 }
#define POWER_SELECT_AUX_VSENSE_ADDR \
  { GPIO_PORT_A, 0 }

// Will go high on fault
#define POWER_SELECT_DCDC_FAULT_ADDR \
  { GPIO_PORT_B, 0 }

// Pull low to trigger shutdown of LTC.  Pulled high through HW
#define POWER_SELECT_LTC_SHDN_ADDR \
  { GPIO_PORT_B, 15 }

// Enables the LTC prioritization.  Active-high, should be pulled high through HW
#define POWER_SELECT_LTC_EN_ADDR \
  { GPIO_PORT_B, 8 }

// Show whether power sources are connected, active-low
#define POWER_SELECT_VALID1_ADDR \
  { GPIO_PORT_A, 10 }
#define POWER_SELECT_VALID2_ADDR { GPIO_PORT_A, 9 }
#define POWER_SELECT_VALID3_ADDR { GPIO_PORT_A, 8 }
