#include "pedal_data_tx.h"
#include "ads1015.h"
#include "brake_data.h"
#include "throttle_data.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "pedal_events.h"
#include "soft_timer.h"

PedalData data = {0};
static Ads1015Storage ads1015_storage;

static int16_t brake_position = 0;
static int16_t throttle_position = 0;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();

  // setup ADC readings
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,                   //
    .scl = { .port = GPIO_PORT_B, .pin = 5 },  // figure out later
    .sda = { .port = GPIO_PORT_B, .pin = 5 },  // figure out later
  };
  i2c_init(I2C_PORT_2, &i2c_settings);
  GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 5 };  // CHANGE
  ads1015_init(&ads1015_storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);

  TEST_ASSERT_OK(pedal_data_tx_init(&ads1015_storage));
}

void teardown_test(void) {}

void test_assert_trivial(void) {
  TEST_ASSERT_TRUE(true);
}