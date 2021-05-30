#include <pthread.h>
#include <unistd.h>

#include "can.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"
#include "wait.h"
#include "x86_interrupt.h"

static uint8_t s_num_times_timer_callback_called;
static uint8_t s_num_times_gpio_callback_called;
static uint8_t s_num_times_x86_callback_called;
static GpioAddress s_test_output_pin = { .port = GPIO_PORT_A, .pin = 0 };
static uint8_t s_interrupt_id;
static CanStorage s_can_storage;
static bool s_can_received;

static uint32_t s_tx_id = 0x01;
static uint64_t s_tx_data = 0x1122334455667788;
static size_t s_tx_len = 8;

#define WAIT_INTERVAL_MS 30
#define EXPECTED_TIMER_INTERRUPT_CYCLES 2
#define EXPECTED_TIMES_TIMER_CALLBACK_CALLED 2
#define EXPECTED_GPIO_INTERRUPT_CYCLES 1
#define EXPECTED_TIMES_GPIO_CALLBACK_CALLED 1
#define EXPECTED_x86_INTERRUPT_CYCLES 1
#define EXPECTED_TIMES_x86_CALLBACK_CALLED 1
#define TEST_CAN_DEVICE_ID 0x1

typedef enum {
  TEST_CAN_EVENT_RX = 10,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
} TestCanEvent;

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  LOG_DEBUG("Received a message!\n");

  if (LOG_LEVEL_VERBOSITY <= LOG_LEVEL_DEBUG) {
    LOG_DEBUG("Data:\n\t");
    uint8_t i;
    for (i = 0; i < msg->dlc; i++) {
      uint8_t byte = 0;
      byte = msg->data >> (i * 8);
      LOG_DEBUG("%x ", byte);
    }
    LOG_DEBUG("\n");
  }
  s_can_received = true;
  return STATUS_CODE_OK;
}

static void prv_test_wait_interrupt_callback(SoftTimerId id, void *context) {
  s_num_times_timer_callback_called++;
}

static void prv_test_wait_gpio_thread_callback(const GpioAddress *address, void *context) {
  LOG_DEBUG("CALLBACK CALLED\n");
  s_num_times_gpio_callback_called++;
}

static void prv_test_wait_x86_thread_callback(uint8_t interrupt_id) {
  LOG_DEBUG("X86 CALLBACK CALLED\n");
  s_num_times_x86_callback_called++;
}

void setup_test(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  gpio_it_init();
  s_test_output_pin.port = GPIO_PORT_A;
  s_test_output_pin.pin = 0;
  s_num_times_timer_callback_called = 0;
  s_num_times_gpio_callback_called = 0;
  s_num_times_x86_callback_called = 0;
  s_can_received = false;
}

static void *prv_gpio_interrupt_thread(void *argument) {
  usleep(30);
  LOG_DEBUG("trigger interrupt\n");
  gpio_it_trigger_interrupt(&s_test_output_pin);
  LOG_DEBUG("INTERRUPT DONE\n");
  pthread_exit(NULL);
  return NULL;
}

static void *prv_x86_interrupt_thread(void *argument) {
  usleep(30);
  LOG_DEBUG("trigger interrupt\n");
  x86_interrupt_trigger(s_interrupt_id);
  pthread_exit(NULL);
  return NULL;
}

static void *prv_can_tx(void *argument) {
  usleep(30);
  can_hw_transmit(s_tx_id, false, (uint8_t *)&s_tx_data, s_tx_len);
  pthread_exit(NULL);
  return NULL;
}

static void prv_init_can(void) {
  CanSettings can_settings = {
    .device_id = TEST_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_CAN_EVENT_RX,
    .tx_event = TEST_CAN_EVENT_TX,
    .fault_event = TEST_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_init(&s_can_storage, &can_settings);
  can_register_rx_default_handler(prv_rx_callback, NULL);
}

void teardown_test(void) {}

// Ideally x86 wait() will suspend the thread until only an interrupt of some sorts happens - from
// x86 interrupt, gpio, timer, or CAN In order to test that wait() suspends properly, the idea is to
// have the main thread suspend until a threshold is met if wait() wakes up prematurely or not at
// all, then a counter is incremented or the thread will hang indefinitely

// Another thread will sleep a certain amount of time before calling an interrupt, and then the
// number of waits done is checked

void test_wait_works_timer(void) {
  uint8_t num_wait_cycles_timer = 0;

  while (s_num_times_timer_callback_called < EXPECTED_TIMES_TIMER_CALLBACK_CALLED) {
    soft_timer_start_millis(WAIT_INTERVAL_MS, prv_test_wait_interrupt_callback, NULL, NULL);

    wait();
    num_wait_cycles_timer++;
  }
  TEST_ASSERT_EQUAL(EXPECTED_TIMER_INTERRUPT_CYCLES, num_wait_cycles_timer);
  TEST_ASSERT_EQUAL(EXPECTED_TIMES_TIMER_CALLBACK_CALLED, s_num_times_timer_callback_called);
}

void test_wait_works_gpio(void) {
  uint8_t num_wait_cycles_timer = 0;
  pthread_t gpio_thread;

  s_test_output_pin.port = GPIO_PORT_A;
  s_test_output_pin.pin = 0;
  static InterruptSettings s_it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };

  gpio_it_register_interrupt(&s_test_output_pin, &s_it_settings, INTERRUPT_EDGE_FALLING,
                             prv_test_wait_gpio_thread_callback, NULL);
  LOG_DEBUG("CREATING THREADS\n");
  pthread_create(&gpio_thread, NULL, prv_gpio_interrupt_thread, NULL);

  while (s_num_times_gpio_callback_called < EXPECTED_TIMES_GPIO_CALLBACK_CALLED) {
    LOG_DEBUG("WAITING: %i\n", s_num_times_gpio_callback_called);
    wait();
    num_wait_cycles_timer++;
  }

  pthread_join(gpio_thread, NULL);
  LOG_DEBUG("JOINED THREADS: %i\n", s_num_times_gpio_callback_called);

  TEST_ASSERT_EQUAL(EXPECTED_GPIO_INTERRUPT_CYCLES, num_wait_cycles_timer);
  TEST_ASSERT_EQUAL(EXPECTED_TIMES_GPIO_CALLBACK_CALLED, s_num_times_gpio_callback_called);
}

void test_wait_works_raw_x86(void) {
  uint8_t num_wait_cycles_timer = 0;
  pthread_t interrupt_thread;

  uint8_t handler_id;

  x86_interrupt_register_handler(prv_test_wait_x86_thread_callback, &handler_id);
  InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };

  x86_interrupt_register_interrupt(handler_id, &it_settings, &s_interrupt_id);

  LOG_DEBUG("CREATING THREADS\n");
  LOG_DEBUG("PIN: %d\n", s_test_output_pin.pin);
  pthread_create(&interrupt_thread, NULL, prv_x86_interrupt_thread, NULL);

  while (s_num_times_x86_callback_called < EXPECTED_TIMES_x86_CALLBACK_CALLED) {
    LOG_DEBUG("WAITING: %i\n", s_num_times_x86_callback_called);
    wait();
    num_wait_cycles_timer++;
  }

  pthread_join(interrupt_thread, NULL);
  LOG_DEBUG("JOINED THREADS: %i\n", s_num_times_x86_callback_called);

  TEST_ASSERT_EQUAL(EXPECTED_x86_INTERRUPT_CYCLES, num_wait_cycles_timer);
  TEST_ASSERT_EQUAL(EXPECTED_TIMES_x86_CALLBACK_CALLED, s_num_times_x86_callback_called);
}

void test_can_wake_works(void) {
  uint8_t num_wait_cycles_timer = 0;

  prv_init_can();

  pthread_t can_send_thread;

  pthread_create(&can_send_thread, NULL, prv_can_tx, NULL);
  Event e = { 0 };
  while (!s_can_received) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
    wait();

    num_wait_cycles_timer++;
    if (num_wait_cycles_timer > 10) break;
  }

  pthread_join(can_send_thread, NULL);

  // we should only wait once
  TEST_ASSERT_EQUAL(EXPECTED_x86_INTERRUPT_CYCLES, num_wait_cycles_timer);
}
