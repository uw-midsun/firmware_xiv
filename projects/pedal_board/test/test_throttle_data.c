#include "ads1015.h"
#include "brake_data.h"
#include "can_transmit.h"
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

static Ads1015Storage ads1015_storage = { 0 };
const PedalDataStorage pedal_data_storage = {
  .storage = &ads1015_storage,
  .throttle_channel1 = ADS1015_CHANNEL_0,
  .throttle_channel2 = ADS1015_CHANNEL_1,
  .brake_channel = ADS1015_CHANNEL_2,
};

static CanStorage can_storage = { 0 };
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

StatusCode prv_test_pedal_data_tx_callback_handler(const CanMessage *msg, void *context,
                                                   CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT, msg->msg_id);
  LOG_DEBUG("IS CALLED\n");
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  can_init(&can_storage, &can_settings);
  // setup ADC readings
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 5 },
    .sda = { .port = GPIO_PORT_B, .pin = 5 },
  };
  i2c_init(I2C_PORT_2, &i2c_settings);
  GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 5 };
  ads1015_init(&ads1015_storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT, prv_test_pedal_data_tx_callback_handler,
                          NULL);
  TEST_ASSERT_OK(pedal_data_tx_init(&pedal_data_storage));
}

void teardown_test(void) {}

void test_assert_trivial(void) {
  TEST_ASSERT_TRUE(true);
}

void test_throttle_data(void) {
  MS_TEST_HELPER_CAN_TX_RX(PEDAL_CAN_TX, PEDAL_CAN_RX);
  int16_t throttle_data = INT16_MAX;
  int16_t throttle_position = get_throttle_position();
  TEST_ASSERT_OK(get_throttle_data(&pedal_data_storage, &throttle_data));
  TEST_ASSERT_EQUAL(throttle_data, throttle_position);
}
