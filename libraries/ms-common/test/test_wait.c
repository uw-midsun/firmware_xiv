#include "wait.h"

#include <pthread.h>
#include <unistd.h>

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"
#include "x86_interrupt.h"

static uint8_t s_num_times_timer_callback_called;
static uint8_t s_num_times_gpio_callback_called;
static uint8_t s_num_times_x86_callback_called;
static GpioAddress s_test_output_pin = { .port = GPIO_PORT_A, .pin = 0 };
static uint8_t interrupt_id;

#define WAIT_INTERVAL_MS 30
#define EXPECTED_TIMER_INTERRUPT_CYCLES 2
#define EXPECTED_TIMES_TIMER_CALLBACK_CALLED 2
#define EXPECTED_GPIO_INTERRUPT_CYCLES 1
#define EXPECTED_TIMES_GPIO_CALLBACK_CALLED 1
#define EXPECTED_x86_INTERRUPT_CYCLES 1
#define EXPECTED_TIMES_x86_CALLBACK_CALLED 1

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
  interrupt_init();
  soft_timer_init();
  gpio_it_init();
  s_test_output_pin.port = GPIO_PORT_A;
  s_test_output_pin.pin = 0;
  s_num_times_timer_callback_called = 0;
  s_num_times_gpio_callback_called = 0;
  s_num_times_x86_callback_called = 0;
}

static void *gpio_interrupt_thread(void *argument) {
  usleep(30);
  LOG_DEBUG("trigger interrupt\n");
  gpio_it_trigger_interrupt(&s_test_output_pin);
  LOG_DEBUG("INTERRUPT DONE\n");
  pthread_exit(NULL);
}

static void *x86_interrupt_thread(void *argument) {
  usleep(30);
  LOG_DEBUG("trigger interrupt\n");
  x86_interrupt_trigger(interrupt_id);
  // s_num_times_gpio_callback_called++;
  pthread_exit(NULL);
}

static void *x86_wake_thread(void *argument) {
  usleep(999999);
  LOG_DEBUG("trigger interrupt\n");
  x86_interrupt_wake();
  // s_num_times_gpio_callback_called++;
  pthread_exit(NULL);
}

void teardown_test(void) {}

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
  pthread_create(&gpio_thread, NULL, gpio_interrupt_thread, NULL);

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
  // uint8_t interrupt_id;

  x86_interrupt_register_interrupt(handler_id, &it_settings, &interrupt_id);

  LOG_DEBUG("CREATING THREADS\n");
  LOG_DEBUG("PIN: %d\n", s_test_output_pin.pin);
  pthread_create(&interrupt_thread, NULL, x86_interrupt_thread, NULL);

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

void test_wait_works_wake(void) {
  // uint8_t num_wait_cycles_timer = 0;
  pthread_t interrupt_thread;

  // uint8_t handler_id;

  // x86_interrupt_register_handler(prv_test_wait_x86_thread_callback, &handler_id);
  // InterruptSettings it_settings = {
  //   .type = INTERRUPT_TYPE_INTERRUPT,       //
  //   .priority = INTERRUPT_PRIORITY_NORMAL,  //
  // };
  // // uint8_t interrupt_id;

  // x86_interrupt_register_interrupt(handler_id, &it_settings, &interrupt_id);

  // LOG_DEBUG("CREATING THREADS\n");
  // LOG_DEBUG("PIN: %d\n", s_test_output_pin.pin);
  pthread_create(&interrupt_thread, NULL, x86_wake_thread, NULL);

  // while (s_num_times_x86_callback_called < EXPECTED_TIMES_x86_CALLBACK_CALLED) {
  LOG_DEBUG("WAITING FOR WAKE \n");
  wait();
  LOG_DEBUG("WAITed\n");
  //   num_wait_cycles_timer++;
  // }

  pthread_join(interrupt_thread, NULL);
  // LOG_DEBUG("JOINED THREADS: %i\n", s_num_times_x86_callback_called);

  // TEST_ASSERT_EQUAL(EXPECTED_x86_INTERRUPT_CYCLES, num_wait_cycles_timer);
  // TEST_ASSERT_EQUAL(EXPECTED_TIMES_x86_CALLBACK_CALLED, s_num_times_x86_callback_called);
}
