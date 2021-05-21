#pragma once

// Pin defintions + other info for power select

#define A_TO_MA 1000
#define V_TO_MV 1000

// Scaling factors
#define POWER_SELECT_PWR_SUP_ISENSE_SCALING 80   // mV/A
#define POWER_SELECT_PWR_SUP_VSENSE_SCALING 190  // mV/V
#define POWER_SELECT_DCDC_ISENSE_SCALING 80      // mV/A
#define POWER_SELECT_DCDC_VSENSE_SCALING 190     // mV/V
#define POWER_SELECT_AUX_ISENSE_SCALING 1        // mV/A
#define POWER_SELECT_AUX_VSENSE_SCALING 190      // mV/V
#define POWER_SELECT_CELL_VSENSE_SCALING 1000    // mV/V (no scaling)

#define POWER_SELECT_VSENSE_SCALING 190  // mV/V
#define POWER_SELECT_ISENSE_SCALING 80   // mV/A

#define POWER_SELECT_AUX_VREF_MV 1624

// Maximum measurements.  Values higher than these represent a fault
#define POWER_SELECT_PWR_SUP_MAX_CURRENT_MA 37500
#define POWER_SELECT_PWR_SUP_MAX_VOLTAGE_MV 15820
#define POWER_SELECT_DCDC_MAX_CURRENT_MA 20000
#define POWER_SELECT_DCDC_MAX_VOLTAGE_MV 15820
#define POWER_SELECT_AUX_MAX_CURRENT_MA 8220
#define POWER_SELECT_AUX_MAX_VOLTAGE_MV 15820

// Cell needs to be above 2V to work, so setting cutoff at 2100 mV
#define POWER_SELECT_CELL_MIN_VOLTAGE_MV 2100

// Offsets for use in fault bitset
// Note: OV: OVERVOLTAGE, OC: OVERCURRENT
// DCDC_PIN is caused by POWER_SELECT_DCDC_FAULT_ADDR going high
typedef enum {
  POWER_SELECT_FAULT_AUX_OV = 0,
  POWER_SELECT_FAULT_DCDC_OV,
  POWER_SELECT_FAULT_PWR_SUP_OV,
  POWER_SELECT_FAULT_AUX_OC,
  POWER_SELECT_FAULT_DCDC_OC,
  POWER_SELECT_FAULT_PWR_SUP_OC,
  POWER_SELECT_FAULT_DCDC_PIN,
  NUM_POWER_SELECT_FAULTS,
} PowerSelectFault;

// Offsets for use in warning bitset
typedef enum {
  POWER_SELECT_WARNING_BAT_LOW = 0,
  NUM_POWER_SELECT_WARNINGS,
} PowerSelectWarning;

// Faults that require the LTC to be turned off
// See https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/1055326209/Power+Selector+Board
#define POWER_SELECT_LTC_DISABLE_FAULT_MASK \
  ((1 << POWER_SELECT_FAULT_AUX_OC) | (1 << POWER_SELECT_FAULT_PWR_SUP_OC))

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
#define POWER_SELECT_CELL_VSENSE_ADDR \
  { GPIO_PORT_B, 1 }

// Will go high on fault
#define POWER_SELECT_DCDC_FAULT_ADDR \
  { GPIO_PORT_B, 0 }

// Pull low to trigger shutdown of LTC.  Pulled high through HW
#define POWER_SELECT_LTC_SHDN_ADDR \
  { GPIO_PORT_B, 15 }

// Enables the LTC prioritization.  Active-high, pulled high through HW.
// Should not be pulled low in any scenario
#define POWER_SELECT_LTC_EN_ADDR \
  { GPIO_PORT_B, 8 }

// Show whether power sources are connected, active-low
#define POWER_SELECT_PWR_SUP_VALID_ADDR \
  { GPIO_PORT_A, 10 }
#define POWER_SELECT_DCDC_VALID_ADDR \
  { GPIO_PORT_A, 9 }
#define POWER_SELECT_AUX_VALID_ADDR \
  { GPIO_PORT_A, 8 }

typedef enum {
  POWER_SELECT_AUX_VALID = 0,
  POWER_SELECT_DCDC_VALID,
  POWER_SELECT_PWR_SUP_VALID,
  NUM_POWER_SELECT_VALID_PINS,
} PowerSelectValidPin;
