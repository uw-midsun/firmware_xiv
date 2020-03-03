#include "ads1015.h"
#include "brake_data.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "pedal_calib.h"
#include "pedal_shared_resources_provider.h"
#include "pedal_data_tx.h"
#include "pedal_events.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "throttle_data.h"

int16_t changeable_value = 0;

StatusCode TEST_MOCK(ads1015_read_raw)(Ads1015Storage *storage, Ads1015Channel channel,
                                       int16_t *reading) {
  *reading = changeable_value;
  return STATUS_CODE_OK;
}

static PedalCalibBlob s_calib_blob = {
  .throttle_calib.upper_value = 100,
  .throttle_calib.lower_value = 0,
  .brake_calib.upper_value = 100,
  .brake_calib.lower_value = 0,
};

static Ads1015Storage s_ads1015_storage = { 0 };
const PedalDataTxStorage pedal_data_storage = {
  .throttle_channel1 = ADS1015_CHANNEL_0,
  .throttle_channel2 = ADS1015_CHANNEL_1,
  .brake_channel = ADS1015_CHANNEL_2,
};

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = PEDAL_CAN_RX,
  .tx_event = PEDAL_CAN_TX,
  .fault_event = PEDAL_CAN_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  // this is need for x86 but not for the stm32s
  flash_init();

  can_init(&s_can_storage, &can_settings);
  // setup ADC readings
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 5 },
    .sda = { .port = GPIO_PORT_B, .pin = 5 },
  };
  i2c_init(I2C_PORT_2, &i2c_settings);
  GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 5 };
  ads1015_init(&s_ads1015_storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);

  TEST_ASSERT_OK(pedal_resources_init(&s_ads1015_storage, &s_calib_blob));
}

void teardown_test(void) {}

void test_brake_data(void) {
  int16_t brake_data = INT16_MAX;
  TEST_ASSERT_OK(get_brake_data(&pedal_data_storage, &brake_data));
  TEST_ASSERT_EQUAL(brake_data, (int16_t)(changeable_value * EE_PEDAL_VALUE_DENOMINATOR));
  changeable_value = 15;
  TEST_ASSERT_OK(get_brake_data(&pedal_data_storage, &brake_data));
  TEST_ASSERT_EQUAL(brake_data, (int16_t)(changeable_value * EE_PEDAL_VALUE_DENOMINATOR));
  changeable_value = 25;
  TEST_ASSERT_OK(get_brake_data(&pedal_data_storage, &brake_data));
  TEST_ASSERT_EQUAL(brake_data, (int16_t)(changeable_value * EE_PEDAL_VALUE_DENOMINATOR));
  changeable_value = 0;
  TEST_ASSERT_OK(get_brake_data(&pedal_data_storage, &brake_data));
  TEST_ASSERT_EQUAL(brake_data, (int16_t)(changeable_value * EE_PEDAL_VALUE_DENOMINATOR));
}
