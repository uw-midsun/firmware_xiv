#include "smoke_current_measurement.h"

#include <stdbool.h>

#include "adc.h"
#include "bug.h"
#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "output.h"
#include "output_config.h"
#include "pin_defs.h"
#include "soft_timer.h"

// Current measurement smoke test: read from all the BTS7200s, BTS7040s, BTS7004s, etc and
// write all of their current sense values to minicom.

// Set to true to use front PD, false to use rear PD
#define USE_FRONT_PD false

// Delay between minicom dumps of current measurement data
#define MEASUREMENT_DELAY_MS 1000

// If you want to only measure current from certain outputs, uncomment the outputs you want to
// read from. If this array is empty, we output everything; otherwise, we only output the currents
// for the outputs in this array.
// Note that we can't measure front outputs when initialized to rear or vice versa.
static Output s_outputs_to_measure[] = {
  // FRONT_OUTPUT_CENTRE_CONSOLE,
  // FRONT_OUTPUT_PEDAL,
  // FRONT_OUTPUT_STEERING,
  // FRONT_OUTPUT_DRIVER_DISPLAY,
  // FRONT_OUTPUT_INFOTAINMENT_DISPLAY,
  // FRONT_OUTPUT_LEFT_DISPLAY,
  // FRONT_OUTPUT_RIGHT_DISPLAY,
  // FRONT_OUTPUT_REAR_DISPLAY,
  // FRONT_OUTPUT_LEFT_CAMERA,
  // FRONT_OUTPUT_RIGHT_CAMERA,
  // FRONT_OUTPUT_MAIN_PI,
  // FRONT_OUTPUT_SPEAKER,
  // FRONT_OUTPUT_FAN,
  // FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT,
  // FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT,
  // FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS,
  // FRONT_OUTPUT_HORN,
  // FRONT_OUTPUT_UV_VBAT,
  // FRONT_OUTPUT_5V_SPARE_1,
  // FRONT_OUTPUT_5V_SPARE_2,
  // FRONT_OUTPUT_SPARE_1,
  // FRONT_OUTPUT_SPARE_2,
  // FRONT_OUTPUT_SPARE_3,
  // FRONT_OUTPUT_SPARE_4,
  // FRONT_OUTPUT_SPARE_5,
  // FRONT_OUTPUT_SPARE_6,
  // REAR_OUTPUT_BMS,
  // REAR_OUTPUT_MCI,
  // REAR_OUTPUT_CHARGER,
  // REAR_OUTPUT_SOLAR_SENSE,
  // REAR_OUTPUT_REAR_CAMERA,
  // REAR_OUTPUT_FAN_1,
  // REAR_OUTPUT_FAN_2,
  // REAR_OUTPUT_LEFT_REAR_TURN_LIGHT,
  // REAR_OUTPUT_RIGHT_REAR_TURN_LIGHT,
  // REAR_OUTPUT_BRAKE_LIGHT,
  // REAR_OUTPUT_BPS_STROBE_LIGHT,
  // REAR_OUTPUT_5V_SPARE_1,
  // REAR_OUTPUT_5V_SPARE_2,
  // REAR_OUTPUT_SPARE_1,
  // REAR_OUTPUT_SPARE_2,
  // REAR_OUTPUT_SPARE_3,
  // REAR_OUTPUT_SPARE_4,
  // REAR_OUTPUT_SPARE_5,
  // REAR_OUTPUT_SPARE_6,
  // REAR_OUTPUT_SPARE_7,
  // REAR_OUTPUT_SPARE_8,
  // REAR_OUTPUT_SPARE_9,
  // REAR_OUTPUT_SPARE_10,
};

static void prv_measure_output(Output output, bool warn_on_unsupported) {
  if (IS_FRONT_OUTPUT(output) != USE_FRONT_PD) {
    LOG_WARN("Cannot read from output '%s' (%d) while initialized as %s!\n", g_output_names[output],
             output, USE_FRONT_PD ? "front" : "rear");
    return;
  }

  uint16_t current;
  StatusCode code = output_read_current(output, &current);
  if (code == STATUS_CODE_INVALID_ARGS) {
    // current measurement isn't supported
    if (warn_on_unsupported) {
      LOG_WARN("Output '%s' (%d) does not support current measurement!\n", g_output_names[output],
               output);
    }
    return;
  } else if (!status_ok(code)) {
    LOG_WARN("Unable to read from output '%s' (%d): status code %d\n", g_output_names[output],
             output, code);
    return;
  }

  LOG_DEBUG("%s (%d): %d mA\n", g_output_names[output], output, current);
}

static void prv_output_everything(void) {
  for (Output output = 0; output < NUM_OUTPUTS; output++) {
    if (IS_FRONT_OUTPUT(output) == USE_FRONT_PD) {
      prv_measure_output(output, false);
    }
  }
}

static void prv_output_from_array(Output outputs[], size_t num_outputs) {
  for (size_t i = 0; i < num_outputs; i++) {
    prv_measure_output(outputs[i], true);
  }
}

static void prv_output_measurements(void) {
  LOG_DEBUG("------\n");
  if (SIZEOF_ARRAY(s_outputs_to_measure) == 0) {
    prv_output_everything();
  } else {
    prv_output_from_array(s_outputs_to_measure, SIZEOF_ARRAY(s_outputs_to_measure));
  }
}

void smoke_current_measurement_perform(void) {
  LOG_DEBUG("Initializing PD current measurement smoke test (%s)\n",
            USE_FRONT_PD ? "front" : "rear");
  BUG(gpio_init());
  interrupt_init();
  soft_timer_init();
  adc_init(ADC_MODE_SINGLE);

  I2CSettings i2c_settings = {
    .scl = PD_I2C_SCL_PIN,
    .sda = PD_I2C_SDA_PIN,
    .speed = I2C_SPEED_FAST,
  };
  BUG(i2c_init(PD_I2C_PORT, &i2c_settings));

  BUG(output_init(&g_combined_output_config, USE_FRONT_PD));

  while (true) {
    prv_output_measurements();
    delay_ms(MEASUREMENT_DELAY_MS);
  }
}
