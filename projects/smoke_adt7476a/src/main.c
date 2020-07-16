#include "adt7476a_fan_controller.h"
#include "adt7476a_fan_controller_defs.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "wait.h"

#define SET_SPEED_INTERVAL 5
#define FAN_SPEED_INCREMENT 10
#define I2C_WRITE_ADDR_1 0x5E
#define I2C_WRITE_ADDR_2 0x58
#define I2C_READ_ADDR_1 0x5F
#define I2C_READ_ADDR_2 0x59

// 2 storages for 2 components
static Adt7476aStorage s_storage_1;
static Adt7476aStorage s_storage_2;
int s_current_speed;

static void prv_periodic_set_speed(SoftTimerId id, void *context) {
  s_current_speed += FAN_SPEED_INCREMENT;
  s_current_speed = s_current_speed % 101;
  LOG_DEBUG("SETTING SPEED: %d PERCENT\n", s_current_speed);
  adt7476a_set_speed(I2C_PORT_2, s_current_speed, ADT_PWM_PORT_1, I2C_WRITE_ADDR_1);
  adt7476a_set_speed(I2C_PORT_2, s_current_speed, ADT_PWM_PORT_2, I2C_WRITE_ADDR_1);

  soft_timer_start_seconds(SET_SPEED_INTERVAL, prv_periodic_set_speed, NULL, NULL);
}

int main() {
  GpioAddress smbalert = {
    .port = GPIO_PORT_A,
    .pin = 9,
  };

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 10 },
    .sda = { .port = GPIO_PORT_B, .pin = 11 },
  };

  const Adt7476aSettings settings_1 = {
    .smbalert_pin = smbalert,
    .callback = NULL,  // set to NULL for no callback
    .callback_context = NULL,
    .i2c_write_addr = I2C_WRITE_ADDR_1,
    .i2c_read_addr = I2C_READ_ADDR_1,
    .i2c = I2C_PORT_2,
    .i2c_settings = i2c_settings,
  };

  const Adt7476aSettings settings_2 = {
    .smbalert_pin = smbalert,
    .callback = NULL,  // set to NULL for no callback
    .callback_context = NULL,
    .i2c_write_addr = I2C_WRITE_ADDR_2,
    .i2c_read_addr = I2C_READ_ADDR_2,
    .i2c = I2C_PORT_2,
    .i2c_settings = i2c_settings,
  };

  interrupt_init();
  gpio_init();
  soft_timer_init();
  adt7476a_init(&s_storage_1, &settings_1);
  adt7476a_init(&s_storage_2, &settings_2);
  s_current_speed = 0;

  soft_timer_start_seconds(SET_SPEED_INTERVAL, prv_periodic_set_speed, NULL, NULL);

  while (true) {
    wait();
  }
}
