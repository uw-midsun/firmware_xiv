#include "ads1015.h"
#include "brake_data.h"
#include "can_transmit.h"
#include "delay.h"
#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
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

static Ads1015Storage s_ads1015_storage = { 0 };
static PedalDataTxStorage pedal_data_storage = {
  .throttle_channel1 = ADS1015_CHANNEL_0,
  .throttle_channel2 = ADS1015_CHANNEL_1,
  .brake_channel = ADS1015_CHANNEL_2,
};

static PedalCalibBlob s_calib_blob = {
  .throttle_calib.upper_value = 100,
  .throttle_calib.lower_value = 0,
  .brake_calib.upper_value = 100,
  .brake_calib.lower_value = 0,
};

static CanStorage s_can_storage = { 0 };
static CanSettings s_can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = PEDAL_CAN_RX,
  .tx_event = PEDAL_CAN_TX,
  .fault_event = PEDAL_CAN_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

int counter = 0;

StatusCode prv_test_pedal_data_tx_callback_handler(const CanMessage *msg, void *context,
                                                   CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT, msg->msg_id);
  counter++;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  can_init(&s_can_storage, &s_can_settings);
  // setup ADC readings
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 5 },
    .sda = { .port = GPIO_PORT_B, .pin = 5 },
  };
  i2c_init(I2C_PORT_2, &i2c_settings);
  GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 5 };
  ads1015_init(&s_ads1015_storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT, prv_test_pedal_data_tx_callback_handler,
                          NULL);

  TEST_ASSERT_OK(pedal_data_init(&s_ads1015_storage, &s_calib_blob));
  TEST_ASSERT_OK(pedal_data_tx_init());
}

void teardown_test(void) {}

void test_pedal_rx_handler(void) {
  // should transmit immediately

  MS_TEST_HELPER_CAN_TX_RX(PEDAL_CAN_TX, PEDAL_CAN_RX);
  int16_t throttle_data = INT16_MAX;
  int16_t brake_data = INT16_MAX;

  changeable_value = 32;
  TEST_ASSERT_EQUAL(counter, 1);
  TEST_ASSERT_OK(get_throttle_data(&pedal_data_storage, &throttle_data));
  TEST_ASSERT_EQUAL(throttle_data, changeable_value);
  changeable_value = 15;
  TEST_ASSERT_OK(get_brake_data(&pedal_data_storage, &brake_data));
  TEST_ASSERT_EQUAL(brake_data, changeable_value);

  delay_ms(100);
  MS_TEST_HELPER_CAN_TX_RX(PEDAL_CAN_TX, PEDAL_CAN_RX);
  TEST_ASSERT_EQUAL(counter, 2);
  changeable_value = 2;
  TEST_ASSERT_OK(get_throttle_data(&pedal_data_storage, &throttle_data));
  TEST_ASSERT_EQUAL(throttle_data, changeable_value);
  changeable_value = 34;
  TEST_ASSERT_OK(get_brake_data(&pedal_data_storage, &brake_data));
  TEST_ASSERT_EQUAL(brake_data, changeable_value);

  delay_ms(95);
  TEST_ASSERT_EQUAL(counter, 2);
  changeable_value = 0;
  TEST_ASSERT_OK(get_throttle_data(&pedal_data_storage, &throttle_data));
  TEST_ASSERT_EQUAL(throttle_data, changeable_value);
  changeable_value = 3;
  TEST_ASSERT_OK(get_brake_data(&pedal_data_storage, &brake_data));
  TEST_ASSERT_EQUAL(brake_data, changeable_value);
  delay_ms(5);
  MS_TEST_HELPER_CAN_TX_RX(PEDAL_CAN_TX, PEDAL_CAN_RX);

  delay_ms(100);
  MS_TEST_HELPER_CAN_TX_RX(PEDAL_CAN_TX, PEDAL_CAN_RX);
  delay_ms(100);
  MS_TEST_HELPER_CAN_TX_RX(PEDAL_CAN_TX, PEDAL_CAN_RX);
  delay_ms(100);
  MS_TEST_HELPER_CAN_TX_RX(PEDAL_CAN_TX, PEDAL_CAN_RX);
  TEST_ASSERT_EQUAL(counter, 6);
  changeable_value = 33;
  TEST_ASSERT_OK(get_throttle_data(&pedal_data_storage, &throttle_data));
  TEST_ASSERT_EQUAL(throttle_data, changeable_value);
  TEST_ASSERT_OK(get_brake_data(&pedal_data_storage, &brake_data));
  TEST_ASSERT_EQUAL(brake_data, changeable_value);

  delay_ms(75);
  TEST_ASSERT_EQUAL(counter, 6);
  delay_ms(25);
  MS_TEST_HELPER_CAN_TX_RX(PEDAL_CAN_TX, PEDAL_CAN_RX);
  TEST_ASSERT_EQUAL(counter, 7);
  changeable_value = 23;
  TEST_ASSERT_OK(get_throttle_data(&pedal_data_storage, &throttle_data));
  TEST_ASSERT_EQUAL(throttle_data, changeable_value);
  changeable_value = 21;
  TEST_ASSERT_OK(get_brake_data(&pedal_data_storage, &brake_data));
  TEST_ASSERT_EQUAL(brake_data, changeable_value);
}
