#include "fan_control.h"

#include "bms.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"

#define FAN_SPEED_POLL_INTERVAL_MS 1000
#define I2C_WRITE_ADDR 0x5E
#define I2C_READ_ADDR 0x5F
#define I2C_PORT I2C_PORT_2
#define ADT_PWM_PORT ADT_PWM_PORT_1
#define VALID_TEMP 21
#define MAX_TEMP 43
#define VALID_SPEED 48
#define SDA \
  { GPIO_PORT_B, 11 }
#define SCL \
  { GPIO_PORT_B, 10 }

static FanStorage s_fan_storage = { 0 };
static AfeReadings s_readings;

StatusCode TEST_MOCK(prv_measure_temps)(SoftTimerId timer_id, void *context) {
  FanStorage *storage = (FanStorage *)context;

  uint16_t max = 0;
  uint8_t fan_speed = 0;

  for (int i = 0; i < NUM_THERMISTORS; i++) {
    if (storage->readings->temps[i] > max) {
      max = storage->readings->temps[i];
    }
  }

  // calculate fan speed

  (max > MAX_BATTERY_TEMP) ? (fan_speed = MAX_FAN_SPEED)
                           : (fan_speed = (MAX_FAN_SPEED / MAX_BATTERY_TEMP));

  adt7476a_set_speed(BMS_FAN_CTRL_I2C_PORT_1, fan_speed, ADT_PWM_PORT_1, storage->i2c_write_addr);
  adt7476a_set_speed(BMS_FAN_CTRL_I2C_PORT_1, fan_speed, ADT_PWM_PORT_2, storage->i2c_write_addr);

  storage->speed = fan_speed;
  storage->status = STATUS_CODE_OK;

  return storage->status;
}

void setup_test(void) {
  soft_timer_init();
}

void teardown_test(void) {}

void test_fan_control_init(void) {
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = SCL,
    .sda = SDA,
  };

  s_fan_storage.i2c_read_addr = I2C_WRITE_ADDR;
  s_fan_storage.i2c_write_addr = I2C_READ_ADDR;

  for (int i = 0; i < NUM_THERMISTORS; i++) {
    s_readings.temps[i] = VALID_TEMP;
  }

  s_fan_storage.readings = &s_readings;

  FanControlSettings s_settings = {
    .i2c_settings = i2c_settings,
    .poll_interval_ms = FAN_SPEED_POLL_INTERVAL_MS,
  };

  TEST_ASSERT_OK(fan_control_init(&s_settings, &s_fan_storage));

  TEST_ASSERT_EQUAL(VALID_SPEED, s_fan_storage.speed);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_fan_storage.status);
}

void test_fan_max_temp(void) {
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = SCL,
    .sda = SDA,
  };

  for (int i = 0; i < NUM_THERMISTORS; i++) {
    s_readings.temps[i] = MAX_TEMP;
  }

  FanControlSettings s_settings = {
    .i2c_settings = i2c_settings,
    .poll_interval_ms = FAN_SPEED_POLL_INTERVAL_MS,
  };

  TEST_ASSERT_OK(fan_control_init(&s_settings, &s_fan_storage));

  TEST_ASSERT_EQUAL(MAX_FAN_SPEED, s_fan_storage.speed);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_fan_storage.status);
}
