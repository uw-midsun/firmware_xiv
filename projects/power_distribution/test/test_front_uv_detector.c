#include "adc.h"
#include "can.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "delay.h"
#include "front_uv_detector.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "pd_events.h"
#include "pd_fan_ctrl.h"
#include "pin_defs.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

static GpioAddress s_uv_comp_pin_address = { .port = GPIO_PORT_B, .pin = 0 };

static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,       //
  .priority = INTERRUPT_PRIORITY_NORMAL,  //
};

static volatile bool s_interrupt_ran = false;

static void prv_test_callback(const GpioAddress *address, void *context) {
  s_interrupt_ran = true;
}

void setup_test(void) {
  gpio_init();
  // status_ok_or_return(interrupt_init());
  // status_ok_or_return(soft_timer_init());
  // status_ok_or_return(event_queue_init());
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  front_uv_detector_init();

  TEST_ASSERT_OK(gpio_it_register_interrupt(&s_uv_comp_pin_address, &s_interrupt_settings,
                                            INTERRUPT_EDGE_FALLING, prv_test_callback, NULL));
  s_interrupt_ran = false;
}

void teardown_test(void) {}

// test that can and log message get sent after lockout
void test_uv_front_detector_notification() {
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_uv_comp_pin_address));
  TEST_ASSERT_TRUE(s_interrupt_ran);
}
