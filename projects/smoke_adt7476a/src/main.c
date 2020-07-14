#include "adt7476a_fan_controller.h"
#include "adt7476a_fan_controller_defs.h"
#include "i2c.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define SET_SPEED_INTERVAL 5
#define I2C_WRITE_ADDR_1
#define I2C_WRITE_ADDR_2
static Adt7476aStorage s_storage;

static void prv_periodic_set_speed(SoftTimerId id, void *context) {

    adt7476a_set_speed(I2C_PORT_1, )

}

int main() {
  
  GpioAddress smbalert = {
      .pin = 10,
  };

  GpioAddress sda = {
      .pin = 10,
  };

  GpioAddress scl = {
      .pin = 10,
  };

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,         //
    .sda = sda, //
    .scl = scl,  //
  };

  const Adt7476aSettings settings = {
    .smbalert_pin = smbalert,
    .callback = NULL,  // set to NULL for no callback
    .callback_context = NULL,
    .i2c = ,
    .i2c_settings =,
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