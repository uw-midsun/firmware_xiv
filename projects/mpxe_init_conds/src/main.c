#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "pca9539r_gpio_expander.h"
#include "soft_timer.h"
#include "wait.h"

// Used for MPXE only, to demo initial conditions setting from harness
// Logs states of pins from initialization

#define PCA9539_I2C_ADDRESS 0x74   // PCA9539 address
#define PCA9539_I2C_ADDRESS2 0x75  // PCA9539 address 2
#define I2C_PORT I2C_PORT_2

// I2C_PORT_1 has SDA on PB9 and SCL on PB8
// I2C_PORT_2 has SDA on PB11 and SCL on PB10

#define PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

static void prv_setup_test(void) {
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = PIN_I2C_SDA,
    .scl = PIN_I2C_SCL,
  };
  i2c_init(I2C_PORT, &i2c_settings);
  pca9539r_gpio_init(I2C_PORT, PCA9539_I2C_ADDRESS);
  pca9539r_gpio_init(I2C_PORT, PCA9539_I2C_ADDRESS2);
}

int main(void) {
  prv_setup_test();
  Pca9539rGpioState in_state;
  Pca9539rGpioAddress address = { .i2c_address = PCA9539_I2C_ADDRESS };
  for (Pca9539rPinAddress pin = PCA9539R_PIN_IO0_0; pin < NUM_PCA9539R_GPIO_PINS; pin++) {
    address.pin = pin;
    pca9539r_gpio_get_state(&address, &in_state);
    LOG_DEBUG("State for First PCA9539R pin %d_%d == %d\n", address.pin / 8, address.pin % 8,
              in_state);
  }
  address.i2c_address = PCA9539_I2C_ADDRESS2;
  for (Pca9539rPinAddress pin = PCA9539R_PIN_IO0_0; pin < NUM_PCA9539R_GPIO_PINS; pin++) {
    address.pin = pin;
    pca9539r_gpio_get_state(&address, &in_state);
    LOG_DEBUG("State for Second PCA9539R pin %d_%d == %d\n", address.pin / 8, address.pin % 8,
              in_state);
  }
  return 0;
}
