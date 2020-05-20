#include "adc.h"
#include "adc_periodic_reader.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "gpio_mcu.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "wait.h"

#define TIMER_INTERVAL_MS 50
static int count = 0;
static bool callback_called = false;

// Function prototypes
void callback_test_1(uint16_t data, PeriodicReaderId id, void *context);
void callback_test_2(uint16_t data, PeriodicReaderId id, void *context);

AdcPeriodicReaderSettings reader_settings1 = { .address = { .port = GPIO_PORT_A, .pin = 3 },
                                               .callback = callback_test_1 };
AdcPeriodicReaderSettings reader_settings_invalid = {
  .address = { .port = NUM_GPIO_PORTS, .pin = 3 }, .callback = callback_test_2
};

AdcPeriodicReaderSettings reader_settings2 = { .address = { .port = GPIO_PORT_A, .pin = 3 },
                                               .callback = callback_test_2 };

void callback_test_1(uint16_t data, PeriodicReaderId id, void *context) {
  count++;
  callback_called = true;
  GpioAddress *address = (GpioAddress *)context;
  TEST_ASSERT_EQUAL(address->pin, reader_settings1.address.pin);
  TEST_ASSERT_EQUAL(address->port, reader_settings1.address.port);
}

void callback_test_2(uint16_t data, PeriodicReaderId id, void *context) {
  count++;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  adc_periodic_reader_init(TIMER_INTERVAL_MS);
  adc_init(ADC_MODE_SINGLE);
}

void test_adc_periodic_reader_test_callback() {
  callback_called = false;
  reader_settings1.context = &reader_settings1.address;
  TEST_ASSERT_OK(adc_periodic_reader_set_up_reader(PERIODIC_READER_ID_0, &reader_settings1));
  TEST_ASSERT_OK(adc_periodic_reader_start(PERIODIC_READER_ID_0));
  delay_ms(100);
  TEST_ASSERT_TRUE(callback_called);
}

void test_invalid_ports_and_pins() {
  callback_called = false;
  TEST_ASSERT_NOT_OK(adc_periodic_reader_set_up_reader(NUM_PERIODIC_READER_IDS, &reader_settings1));
  TEST_ASSERT_NOT_OK(adc_periodic_reader_start(NUM_PERIODIC_READER_IDS));
  delay_ms(100);
  TEST_ASSERT_FALSE(callback_called);
}

void test_count_time_callback_runs() {
  TEST_ASSERT_OK(adc_periodic_reader_set_up_reader(PERIODIC_READER_ID_3, &reader_settings2));
  TEST_ASSERT_OK(adc_periodic_reader_start(PERIODIC_READER_ID_3));
  count = 0;
  // Callback should go off approximately every 50 ms
  delay_ms(49);
  TEST_ASSERT_EQUAL(0, count);
  delay_ms(1);
  TEST_ASSERT_EQUAL(1, count);
  // Added 1ms to account for the time needed to increment count
  delay_ms(51 * 3);  // Check count after 4 cycles
  TEST_ASSERT_EQUAL(4, count);
  delay_ms(51 * 5);  // Check count after 9 cycles
  TEST_ASSERT_EQUAL(9, count);
}

void teardown_test(void) {}
