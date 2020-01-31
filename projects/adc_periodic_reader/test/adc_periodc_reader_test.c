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

#define TEST_1_ADDRESS \
  { GPIO_PORT_B, 1 }
#define TEST_2_ADDRESS \
  { GPIO_PORT_B, 2 }
#define TEST_3_ADDRESS \
  { GPIO_PORT_B, 3 }

typedef enum { TEST_1 = 0, TEST_2, TEST_3 } test_number;

const GpioSettings settings = {
  .direction = GPIO_DIR_IN,           //
  .state = GPIO_STATE_LOW,            //
  .resistor = GPIO_RES_NONE,          //
  .alt_function = GPIO_ALTFN_ANALOG,  //
};

const AdcPeriodicReaderStorage storage[3] = { [TEST_1].address = TEST_1_ADDRESS,
                                              [TEST_2].address = TEST_2_ADDRESS,
                                              [TEST_3].address = TEST_3_ADDRESS };

void setup_test() {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
}

void teardown_test(void) {}

void test_initialization(void) {
  ASSERT_OK(adc_periodic_reader_init(&storage, &settings));
  ASSERT_OK(adc_periodic_reader_set_up_channel(&storage));
}

/*
Other test that will be made
1) Check that the specificied channel is not NULL
2) Check that the channel has been enabled and is true for each one
3) Try registering the callback and seeing if it will run every 50miliseconds
  (make it print out a voltage)
*/
