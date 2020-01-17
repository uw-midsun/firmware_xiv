#include "ads1015.h"
#include "can.h"
#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
// include all the modules
#include "brake_fsm.h"
#include "pedal_events.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define CAN_DEVICE_ID 0x1

static Fsm brake_fsm;
static Ads1015Storage ads1015_storage;
static CanStorage can_storage;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  const CanSettings can_settings = {
    .device_id = CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = PEDAL_CAN_RX,
    .tx_event = PEDAL_CAN_TX,
    .fault_event = PEDAL_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },  // CHANGE
    .rx = { GPIO_PORT_A, 11 },  // CHANGE
  };
  can_init(&can_storage, &can_settings);

  // setup ADC readings
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,                   //
    .scl = { .port = GPIO_PORT_B, .pin = 5 },  // figure out later
    .sda = { .port = GPIO_PORT_B, .pin = 5 },  // figure out later
  };
  i2c_init(I2C_PORT_2, &i2c_settings);
  GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 5 };  // CHANGE
  ads1015_init(&ads1015_storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);

  TEST_ASSERT_OK( brake_fsm_init(&brake_fsm));
}

void teardown_test(void) {}

void test_assert_trivial(void) {
  TEST_ASSERT_TRUE(true);
}

////////////  RELEASED TO //////////////////////
void test_brake_fsm_released_to_pressed(void) {
  Event e = {
    .id = PEDAL_BRAKE_FSM_EVENT_PRESSED,
  };
  TEST_ASSERT_TRUE(brake_fsm_process_event(&e));
}
void test_brake_fsm_released_to_released(void) {
  Event e = {
    .id = PEDAL_BRAKE_FSM_EVENT_RELEASED,
  };
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_released_to_can_fault(void) {
  Event e = {
    .id = PEDAL_CAN_FAULT,
  };
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_released_to_can_rx(void) {
  Event e = {
    .id = PEDAL_CAN_RX,
  };
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_released_to_can_tx(void) {
  Event e = {
    .id = PEDAL_CAN_TX,
  };
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_released_to_input_fault(void) {
  Event e = {
    .id = PEDAL_DRIVE_INPUT_EVENT_FAULT,
  };
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_released_to_input_neutral(void) {
  Event e = {
    .id = PEDAL_DRIVE_INPUT_EVENT_NEUTRAL,
  };
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_released_to_input_drive(void) {
  Event e = {
    .id = PEDAL_DRIVE_INPUT_EVENT_DRIVE,
  };
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_released_to_can_pressed(void) {
  Event e = {
    .id = PEDAL_CAN_EVENT_BRAKE_PRESSED,
  };
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_released_to_can_released(void) {
  Event e = {
    .id = PEDAL_CAN_EVENT_BRAKE_RELEASED,
  };
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_released_to_throttle(void) {
  Event e = {
    .id = PEDAL_EVENT_THROTTLE_READING,
  };
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}

//////////////////////  PRESSED TO  //////////////////////////////////////////
void test_brake_fsm_pressed_to_released(void) {
  Event e = {
    .id = PEDAL_BRAKE_FSM_EVENT_PRESSED,
  };
  brake_fsm_process_event(&e);

  e.id = PEDAL_BRAKE_FSM_EVENT_RELEASED;
  TEST_ASSERT_TRUE(brake_fsm_process_event(&e));
}
void test_brake_fsm_pressed_to_pressed(void) {
  Event e = {
    .id = PEDAL_BRAKE_FSM_EVENT_PRESSED,
  };
  brake_fsm_process_event(&e);

  e.id = PEDAL_BRAKE_FSM_EVENT_PRESSED;
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_pressed_to_can_fault(void) {
  Event e = {
    .id = PEDAL_BRAKE_FSM_EVENT_PRESSED,
  };
  brake_fsm_process_event(&e);

  e.id = PEDAL_CAN_FAULT;
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_pressed_to_can_rx(void) {
  Event e = {
    .id = PEDAL_BRAKE_FSM_EVENT_PRESSED,
  };
  brake_fsm_process_event(&e);

  e.id = PEDAL_CAN_RX;
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_pressed_to_can_tx(void) {
  Event e = {
    .id = PEDAL_BRAKE_FSM_EVENT_PRESSED,
  };
  brake_fsm_process_event(&e);

  e.id = PEDAL_CAN_TX;
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_pressed_to_input_fault(void) {
  Event e = {
    .id = PEDAL_BRAKE_FSM_EVENT_PRESSED,
  };
  brake_fsm_process_event(&e);

  e.id = PEDAL_DRIVE_INPUT_EVENT_FAULT;
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_pressed_to_input_neutral(void) {
  Event e = {
    .id = PEDAL_BRAKE_FSM_EVENT_PRESSED,
  };
  brake_fsm_process_event(&e);

  e.id = PEDAL_DRIVE_INPUT_EVENT_NEUTRAL;
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_pressed_to_input_drive(void) {
  Event e = {
    .id = PEDAL_BRAKE_FSM_EVENT_PRESSED,
  };
  brake_fsm_process_event(&e);

  e.id = PEDAL_DRIVE_INPUT_EVENT_DRIVE;
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_pressed_to_can_pressed(void) {
  Event e = {
    .id = PEDAL_BRAKE_FSM_EVENT_PRESSED,
  };
  brake_fsm_process_event(&e);

  e.id = PEDAL_CAN_EVENT_BRAKE_PRESSED;
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_pressed_to_can_released(void) {
  Event e = {
    .id = PEDAL_BRAKE_FSM_EVENT_PRESSED,
  };
  brake_fsm_process_event(&e);

  e.id = PEDAL_CAN_EVENT_BRAKE_RELEASED;
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}
void test_brake_fsm_pressed_to_throttle(void) {
  Event e = {
    .id = PEDAL_BRAKE_FSM_EVENT_PRESSED,
  };
  brake_fsm_process_event(&e);

  e.id = PEDAL_EVENT_THROTTLE_READING;
  TEST_ASSERT_FALSE(brake_fsm_process_event(&e));
}

// test prv channel

// test prv output functions
