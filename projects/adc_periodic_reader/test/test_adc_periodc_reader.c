#include "adc.h"
#include "adc_periodic_reader.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"

void callback(uint16_t data, PeriodicReaderId id, void *context) {
  LOG_DEBUG("hi");
}

AdcPeriodicReaderSettings adc_settings =  {
  .address = {.port = GPIO_PORT_A, .pin=1},
  .callback = callback
};


void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  adc_periodic_reader_init();
  adc_init(ADC_MODE_SINGLE);
}

//Ensure all ADC's are disabled
void test_adc_periodic_reader_init() {
  for (size_t i = 0; i < NUM_PERIODIC_READER_IDS; i++) {
    TEST_ASSERT_FALSE(s_storage[i].activated);
  }
}

void test_adc_periodic_reader_set_up_reader() {
  TEST_ASSERT_OK(adc_periodic_reader_set_up_reader(PERIODIC_READER_ID_0,&adc_settings));
  TEST_ASSERT_EQUAL(s_storage[PERIODIC_READER_ID_0].address.pin,adc_settings.address.pin);
  TEST_ASSERT_NOT_EQUAL(s_storage[PERIODIC_READER_ID_1].address.pin,adc_settings.address.pin);
}

void teardown_test(void) {}



