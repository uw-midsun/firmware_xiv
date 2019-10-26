#include "gpio_it.h"

#include <stdint.h>
#include <stdlib.h>

#include "critical_section.h"
#include "gpio.h"
#include "interrupt.h"
#include "interrupt_def.h"
#include "test_helpers.h"
#include "unity.h"

static GpioAddress s_interrupt_address = { 1, 0 };
static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,       //
  .priority = INTERRUPT_PRIORITY_NORMAL,  //
};
static GpioAddress s_event_address = { 1, 1 };
static InterruptSettings s_event_settings = {
  .type = INTERRUPT_TYPE_EVENT,           //
  .priority = INTERRUPT_PRIORITY_NORMAL,  //
};

static volatile bool s_correct_port = false;
static volatile bool s_interrupt_ran = false;

static void prv_test_callback(const GpioAddress *address, void *context) {
  if (address->port == 1) {
    s_correct_port = true;
  }
  s_interrupt_ran = true;
}

void setup_test(void) {
  // Init everything.
  interrupt_init();
  gpio_init();
  gpio_it_init();
  s_correct_port = false;
  s_interrupt_ran = false;
}

void teardown_test(void) {
  // Forcibly enable interrupts.
  critical_section_end(true);
}

// Test interrupt in critical section
void test_gpio_it_interrupt_critical(void) {
  TEST_ASSERT_OK(gpio_it_register_interrupt(&s_interrupt_address, &s_interrupt_settings,
                                            INTERRUPT_EDGE_RISING, prv_test_callback, NULL));

  // Run the interrupt in critical section.
  bool disabled = critical_section_start();
  TEST_ASSERT_TRUE(disabled);
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_interrupt_address));
  TEST_ASSERT_FALSE(s_interrupt_ran);
  critical_section_end(disabled);

  TEST_ASSERT_TRUE(s_interrupt_ran);
  TEST_ASSERT_TRUE(s_correct_port);
}

// End to end interrupt.
void test_gpio_it_interrupt(void) {
  TEST_ASSERT_OK(gpio_it_register_interrupt(&s_interrupt_address, &s_interrupt_settings,
                                            INTERRUPT_EDGE_RISING, prv_test_callback, NULL));
  // Run the interrupt normally.
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_interrupt_address));
  TEST_ASSERT_TRUE(s_interrupt_ran);
  TEST_ASSERT_TRUE(s_correct_port);
}

// End to end event, events wake the board and do not run callbacks.
void test_gpio_it_event(void) {
  TEST_ASSERT_OK(gpio_it_register_interrupt(&s_event_address, &s_event_settings,
                                            INTERRUPT_EDGE_RISING, NULL, NULL));
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_event_address));
}
