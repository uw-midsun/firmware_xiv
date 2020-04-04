// Current measurement testing main

// I haven't figured out how to test if pin 31 is grounded, so as of yet to run on rear power
// distribution change this to false.
#define ON_FRONT_POWER_DISTRIBUTION true

#include "power_distribution_current_measurement.h"
#include "power_distribution_current_measurement_config.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "adc.h"
#include "log.h"

#define I2C_PORT I2C_PORT_2
#define I2C_SCL_PIN { GPIO_PORT_B, 10 }
#define I2C_SDA_PIN { GPIO_PORT_B, 11 }

#define READ_INTERVAL_US 500000

static const char * const s_current_names[] = {
  // Currents for front power distribution
  [FRONT_POWER_DISTRIBUTION_CURRENT_LEFT_CAMERA] = "Left camera",
  [FRONT_POWER_DISTRIBUTION_CURRENT_RIGHT_CAMERA] = "Right camera",
  [FRONT_POWER_DISTRIBUTION_CURRENT_MAIN_DISPLAY] = "Main display",
  [FRONT_POWER_DISTRIBUTION_CURRENT_REAR_DISPLAY] = "Rear display",
  [FRONT_POWER_DISTRIBUTION_CURRENT_DRIVER_DISPLAY] = "Driver display",
  [FRONT_POWER_DISTRIBUTION_CURRENT_LEFT_DRIVER_DISPLAY] = "Left driver display",
  [FRONT_POWER_DISTRIBUTION_CURRENT_RIGHT_DRIVER_DISPLAY] = "Right driver display",
  [FRONT_POWER_DISTRIBUTION_CURRENT_MAIN_PI] = "Main rPi",
  [FRONT_POWER_DISTRIBUTION_CURRENT_REAR_PI] = "Rear rPi",
  [FRONT_POWER_DISTRIBUTION_CURRENT_LEFT_FRONT_TURN_LIGHT] = "Left front turn light",
  [FRONT_POWER_DISTRIBUTION_CURRENT_RIGHT_FRONT_TURN_LIGHT] = "Right front turn light",
  [FRONT_POWER_DISTRIBUTION_CURRENT_DAYTIME_RUNNING_LIGHTS] = "Daytime running lights",
  [FRONT_POWER_DISTRIBUTION_CURRENT_CENTRE_CONSOLE] = "Centre console",
  [FRONT_POWER_DISTRIBUTION_CURRENT_PEDAL] = "Pedal",
  [FRONT_POWER_DISTRIBUTION_CURRENT_STEERING] = "Steering",
  [FRONT_POWER_DISTRIBUTION_CURRENT_PARKING_BRAKE] = "Parking brake",
  [FRONT_POWER_DISTRIBUTION_CURRENT_SPEAKER] = "Speaker",
  [FRONT_POWER_DISTRIBUTION_CURRENT_HORN] = "Horn",
  [FRONT_POWER_DISTRIBUTION_CURRENT_5V_SPARE_1] = "5V Spare 1",
  [FRONT_POWER_DISTRIBUTION_CURRENT_5V_SPARE_2] = "5V Spare 2",
  [FRONT_POWER_DISTRIBUTION_CURRENT_SPARE_1] = "Spare 1",
  [FRONT_POWER_DISTRIBUTION_CURRENT_SPARE_2] = "Spare 2",

  // Currents for rear power distribution
  [REAR_POWER_DISTRIBUTION_CURRENT_BMS_CARRIER] = "BMS Carrier",
  [REAR_POWER_DISTRIBUTION_CURRENT_MCI] = "MCI",
  [REAR_POWER_DISTRIBUTION_CURRENT_CHARGER] = "Charger",
  [REAR_POWER_DISTRIBUTION_CURRENT_SOLAR_SENSE] = "Solar sense",
  [REAR_POWER_DISTRIBUTION_CURRENT_TELEMETRY] = "Telemetry",
  [REAR_POWER_DISTRIBUTION_CURRENT_REAR_CAMERA] = "Rear camera",
  [REAR_POWER_DISTRIBUTION_CURRENT_LEFT_REAR_TURN_LIGHT] = "Left rear turn light",
  [REAR_POWER_DISTRIBUTION_CURRENT_RIGHT_REAR_TURN_LIGHT] = "Right rear turn light",
  [REAR_POWER_DISTRIBUTION_CURRENT_LEFT_BRAKE_LIGHT] = "Left brake light",
  [REAR_POWER_DISTRIBUTION_CURRENT_CENTRE_BRAKE_LIGHT] = "Centre brake light",
  [REAR_POWER_DISTRIBUTION_CURRENT_RIGHT_BRAKE_LIGHT] = "Right brake light",
  [REAR_POWER_DISTRIBUTION_CURRENT_STROBE] = "Strobe",
  [REAR_POWER_DISTRIBUTION_CURRENT_SPARE_1] = "Spare 1",
  [REAR_POWER_DISTRIBUTION_CURRENT_SPARE_2] = "Spare 2",
  [REAR_POWER_DISTRIBUTION_CURRENT_SPARE_3] = "Spare 3",
  [REAR_POWER_DISTRIBUTION_CURRENT_SPARE_4] = "Spare 4",
  [REAR_POWER_DISTRIBUTION_CURRENT_SPARE_5] = "Spare 5",
  [REAR_POWER_DISTRIBUTION_CURRENT_SPARE_6] = "Spare 6",
  [REAR_POWER_DISTRIBUTION_CURRENT_SPARE_7] = "Spare 7",
  [REAR_POWER_DISTRIBUTION_CURRENT_SPARE_8] = "Spare 8",
  [REAR_POWER_DISTRIBUTION_CURRENT_SPARE_9] = "Spare 9",
  [REAR_POWER_DISTRIBUTION_CURRENT_SPARE_10] = "Spare 10",
};

void print_current_measurements(void *context) {
  PowerDistributionCurrentStorage *storage = power_distribution_current_measurement_get_storage();
  
  LOG_DEBUG("--- Begin current measurements on ");
  if (ON_FRONT_POWER_DISTRIBUTION) {
    LOG_DEBUG("front power distribution");
  } else {
    LOG_DEBUG("rear power distribution");
  }
  LOG_DEBUG(" ---\r\n");
  
  // limits here must be updated if the current array changes
  if (ON_FRONT_POWER_DISTRIBUTION) {
    for (PowerDistributionCurrent current = FRONT_POWER_DISTRIBUTION_CURRENT_LEFT_CAMERA;
         current < REAR_POWER_DISTRIBUTION_CURRENT_BMS_CARRIER; current++) {
      LOG_DEBUG("%s", s_current_names[current]);
      LOG_DEBUG(": %d\r\n", storage->measurements[current]);
    }
  } else {
    for (PowerDistributionCurrent current = REAR_POWER_DISTRIBUTION_CURRENT_BMS_CARRIER;
         current < NUM_POWER_DISTRIBUTION_CURRENTS; current++) {
      LOG_DEBUG("%s", s_current_names[current]);
      LOG_DEBUG(": %d\r\n", storage->measurements[current]);
    }
  }
}

int main(void) {
  // initialize everything
  gpio_init();
  interrupt_init();
  soft_timer_init();
  adc_init(ADC_MODE_SINGLE);
  
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = I2C_SCL_PIN,
    .sda = I2C_SDA_PIN,
  };
  i2c_init(I2C_PORT, &i2c_settings);
  
  PowerDistributionCurrentSettings settings = {
    .hw_config = ON_FRONT_POWER_DISTRIBUTION ? FRONT_POWER_DISTRIBUTION_CURRENT_HW_CONFIG
      : REAR_POWER_DISTRIBUTION_CURRENT_HW_CONFIG,
    .interval_us = READ_INTERVAL_US,
    .callback = &print_current_measurements,
  };
  power_distribution_current_measurement_init(&settings);
  
  return 0;
}
