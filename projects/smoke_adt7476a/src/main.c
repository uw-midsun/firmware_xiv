#include <stdio.h>
#include <stdlib.h>
#include "adt7476a_fan_controller.h"
#include "adt7476a_fan_controller_defs.h"
#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define SET_SPEED_INTERVAL 5
#define I2C_WRITE_ADDR_1 0x5E
#define I2C_WRITE_ADDR_2 0x58

static Adt7476aStorage s_storage;

static void prv_periodic_set_speed(SoftTimerId id, void *context) {
  uint8_t random_speed = rand() % 254;

  adt7476a_set_speed(I2C_PORT_2, random_speed, ADT_FAN_GROUP_1, I2C_WRITE_ADDR_1);
  adt7476a_set_speed(I2C_PORT_2, random_speed, ADT_FAN_GROUP_2, I2C_WRITE_ADDR_1);

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

  const Adt7476aSettings settings = {
    .smbalert_pin = smbalert,
    .callback = NULL,  // set to NULL for no callback
    .callback_context = NULL,
    .i2c = I2C_PORT_2,
    .i2c_settings = i2c_settings,
  };
  interrupt_init();
  gpio_init();
  soft_timer_init();
  adt7476a_init(&s_storage, &settings);

  while (1) {
    soft_timer_start_seconds(SET_SPEED_INTERVAL, prv_periodic_set_speed, NULL, NULL);
    delay_ms(1000);
  }
}