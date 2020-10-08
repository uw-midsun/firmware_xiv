#include "led_manager.h"

#include "centre_console_events.h"
#include "centre_console_fault_reason.h"
#include "controller_board_pins.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "i2c.h"
#include "log.h"
#include "mcp23008_gpio_expander.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_I2C_PORT I2C_PORT_2
#define TEST_SCL CONTROLLER_BOARD_ADDR_I2C2_SCL
#define TEST_SDA CONTROLLER_BOARD_ADDR_I2C2_SDA

static Mcp23008GpioAddress *s_led_to_address = NULL;

#define TEST_ASSERT_LED_STATE(led, expected_state)                  \
  ({                                                                \
    Mcp23008GpioState actual_state = NUM_MCP23008_GPIO_STATES;      \
    mcp23008_gpio_get_state(&s_led_to_address[led], &actual_state); \
    TEST_ASSERT_EQUAL(expected_state, actual_state);                \
  })

void setup_test(void) {
  gpio_init();
  event_queue_init();

  I2CSettings i2c_settings = {
    .scl = TEST_SCL,
    .sda = TEST_SDA,
    .speed = I2C_SPEED_FAST,
  };
  i2c_init(TEST_I2C_PORT, &i2c_settings);

  TEST_ASSERT_OK(led_manager_init());
  s_led_to_address = test_led_manager_provide_led_to_address();
}
void teardown_test(void) {}

// Test that all the LEDs are initially low.
void test_all_low_at_start(void) {
  for (CentreConsoleLed led = 0; led < NUM_CENTRE_CONSOLE_LEDS; led++) {
    TEST_ASSERT_LED_STATE(led, MCP23008_GPIO_STATE_LOW);
  }
}

// Test that BPS fault events turn the BPS LED on.
void test_setting_bps_led_on_bps_fault(void) {
  FaultReason bps_reason = { .fields = { .area = EE_CONSOLE_FAULT_AREA_BPS_HEARTBEAT } };
  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_FAULT, .data = bps_reason.raw };
  TEST_ASSERT_TRUE(led_manager_process_event(&e));
  TEST_ASSERT_LED_STATE(CENTRE_CONSOLE_LED_BPS, MCP23008_GPIO_STATE_HIGH);
}

// Test that the BPS LED does nothing on non-BPS faults.
void test_non_bps_fault_no_bps_led(void) {
  FaultReason non_bps_reason = { .fields = {
                                     .area = EE_CONSOLE_FAULT_AREA_BPS_HEARTBEAT + 1,
                                 } };
  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_FAULT, .data = non_bps_reason.raw };
  TEST_ASSERT_TRUE(led_manager_process_event(&e));
  TEST_ASSERT_LED_STATE(CENTRE_CONSOLE_LED_BPS, MCP23008_GPIO_STATE_LOW);
}

// Test that the hazard LED is toggled on hazard on/off events.
void test_hazard_led(void) {
  const Event hazard_on = { .id = HAZARD_EVENT_ON };
  const Event hazard_off = { .id = HAZARD_EVENT_OFF };

  TEST_ASSERT_TRUE(led_manager_process_event(&hazard_on));
  TEST_ASSERT_LED_STATE(CENTRE_CONSOLE_LED_HAZARDS, MCP23008_GPIO_STATE_HIGH);

  TEST_ASSERT_TRUE(led_manager_process_event(&hazard_off));
  TEST_ASSERT_LED_STATE(CENTRE_CONSOLE_LED_HAZARDS, MCP23008_GPIO_STATE_LOW);

  // setting twice in a row has no effect, but we still return true
  TEST_ASSERT_TRUE(led_manager_process_event(&hazard_off));
  TEST_ASSERT_LED_STATE(CENTRE_CONSOLE_LED_HAZARDS, MCP23008_GPIO_STATE_LOW);

  TEST_ASSERT_TRUE(led_manager_process_event(&hazard_on));
  TEST_ASSERT_LED_STATE(CENTRE_CONSOLE_LED_HAZARDS, MCP23008_GPIO_STATE_HIGH);

  // same thing with twice in a row, but this time it's on
  TEST_ASSERT_TRUE(led_manager_process_event(&hazard_on));
  TEST_ASSERT_LED_STATE(CENTRE_CONSOLE_LED_HAZARDS, MCP23008_GPIO_STATE_HIGH);
}

// Test that the power LED is toggled on power sequence completion events.
void test_power_led(void) {
  const Event power_main_sequence_complete = { .id = POWER_MAIN_SEQUENCE_EVENT_COMPLETE };
  const Event power_aux_sequence_complete = { .id = POWER_AUX_SEQUENCE_EVENT_COMPLETE };
  const Event power_off_sequence_complete = { .id = POWER_OFF_SEQUENCE_EVENT_COMPLETE };

  // main complete turns on
  TEST_ASSERT_TRUE(led_manager_process_event(&power_main_sequence_complete));
  TEST_ASSERT_LED_STATE(CENTRE_CONSOLE_LED_POWER, MCP23008_GPIO_STATE_HIGH);

  // off complete turns off
  TEST_ASSERT_TRUE(led_manager_process_event(&power_off_sequence_complete));
  TEST_ASSERT_LED_STATE(CENTRE_CONSOLE_LED_POWER, MCP23008_GPIO_STATE_LOW);

  // aux complete turns on
  TEST_ASSERT_TRUE(led_manager_process_event(&power_aux_sequence_complete));
  TEST_ASSERT_LED_STATE(CENTRE_CONSOLE_LED_POWER, MCP23008_GPIO_STATE_HIGH);

  // and multiple events doing the same thing do what you expect
  TEST_ASSERT_TRUE(led_manager_process_event(&power_main_sequence_complete));
  TEST_ASSERT_LED_STATE(CENTRE_CONSOLE_LED_POWER, MCP23008_GPIO_STATE_HIGH);

  TEST_ASSERT_TRUE(led_manager_process_event(&power_aux_sequence_complete));
  TEST_ASSERT_LED_STATE(CENTRE_CONSOLE_LED_POWER, MCP23008_GPIO_STATE_HIGH);

  TEST_ASSERT_TRUE(led_manager_process_event(&power_off_sequence_complete));
  TEST_ASSERT_LED_STATE(CENTRE_CONSOLE_LED_POWER, MCP23008_GPIO_STATE_LOW);

  TEST_ASSERT_TRUE(led_manager_process_event(&power_off_sequence_complete));
  TEST_ASSERT_LED_STATE(CENTRE_CONSOLE_LED_POWER, MCP23008_GPIO_STATE_LOW);
}

void test_drive_state_leds(void) {
  // TODO(SOFT-296)
}
