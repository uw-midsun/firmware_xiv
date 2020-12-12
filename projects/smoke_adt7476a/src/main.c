#include "adt7476a_fan_controller.h"
#include "adt7476a_fan_controller_defs.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

// this test will increment the fan speed by 10% every second until at full speed,
// and then loop back to 0% speed and continue.

// the speed interval and increment defines can be changed for further testing

// the i2c port, i2c write and read address, and i2c settings should be changed as needed
// depending on the board.

#define ADT_7476A_NUM_FANS 4
#define ADT_7476A_INTERRUPT_MASK_OFFSET 2
#define SET_SPEED_INTERVAL_S 1
#define FAN_SPEED_INCREMENT 10
#define I2C_WRITE_ADDR 0x5E
#define I2C_READ_ADDR 0x5F
#define I2C_PORT I2C_PORT_2
#define ADT_PWM_PORT ADT_PWM_PORT_1
#define SMBALERT_ADDR \
  { GPIO_PORT_A, 9 }
#define SDA \
  { GPIO_PORT_B, 11 }
#define SCL \
  { GPIO_PORT_B, 10 }
static Adt7476aStorage s_storage;

int s_current_speed;

static void prv_periodic_set_speed(SoftTimerId id, void *context) {
  s_current_speed += FAN_SPEED_INCREMENT;
  s_current_speed = s_current_speed % 101;
  LOG_DEBUG("SETTING SPEED: %d PERCENT\n", s_current_speed);
  adt7476a_set_speed(I2C_PORT, s_current_speed, ADT_PWM_PORT, I2C_WRITE_ADDR);
  soft_timer_start_seconds(SET_SPEED_INTERVAL_S, prv_periodic_set_speed, NULL, NULL);
}

static void prv_callback(GpioAddress *address, void *context) {
  LOG_DEBUG("FAN OUT OF RANGE");

  uint8_t rx_interrupt_status_reg_1;
  uint8_t rx_interrupt_status_reg_2;

  uint8_t adt_7476a_fan_status[4];

  adt7476a_get_status(I2C_PORT, I2C_READ_ADDR, &rx_interrupt_status_reg_1,
                      &rx_interrupt_status_reg_2);

  // bit positions 2 - 5 indicate out of range fan speeds for fans 1 - 4
  for (int i = 0; i < ADT_7476A_NUM_FANS; i++) {
    if ((rx_interrupt_status_reg_2 >> (ADT_7476A_INTERRUPT_MASK_OFFSET + i)) && 1) {
      LOG_DEBUG("FAN %d HAS STALLED\n", i + 1);
    }
  }
}

int main() {
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = SCL,
    .sda = SDA,
  };

  const Adt7476aSettings settings = {
    .smbalert_pin = SMBALERT_ADDR,
    .callback = (GpioItCallback)prv_callback,  // set to NULL for no callback
    .callback_context = NULL,
    .i2c_write_addr = I2C_WRITE_ADDR,
    .i2c_read_addr = I2C_READ_ADDR,
    .i2c = I2C_PORT,
  };

  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();
  adt7476a_init(&s_storage, &settings);
  i2c_init(I2C_PORT, &i2c_settings);

  s_current_speed = 0;

  prv_periodic_set_speed(SOFT_TIMER_INVALID_TIMER, NULL);
  while (true) {
    LOG_DEBUG("WAITING\n");
    wait();
  }
}
