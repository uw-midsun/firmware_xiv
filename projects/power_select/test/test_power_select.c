#include "power_select.h"
#include "power_select_events.h"

#include "can_msg_defs.h"
#include "delay.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "string.h"
#include "test_helpers.h"

#define TEST_TEMP_VOLTAGE_MV 1100

#define TEST_GOOD_VOLTAGE_MV 3000
#define TEST_GOOD_CURRENT_MA 4000

// Account for scaling
#define TEST_GOOD_VOLTAGE_SCALED_MV (TEST_GOOD_VOLTAGE_MV * POWER_SELECT_VSENSE_SCALING / V_TO_MV)
#define TEST_GOOD_CURRENT_SCALED_MA (TEST_GOOD_CURRENT_MA * POWER_SELECT_ISENSE_SCALING / A_TO_MA)

#define TEST_FAULT_VOLTAGE_MV 20000
#define TEST_FAULT_CURRENT_MA 40000

// Account for scaling
#define TEST_FAULT_VOLTAGE_SCALED_MV (TEST_FAULT_VOLTAGE_MV * POWER_SELECT_VSENSE_SCALING / V_TO_MV)
#define TEST_FAULT_CURRENT_SCALED_MA (TEST_FAULT_CURRENT_MA * POWER_SELECT_ISENSE_SCALING / A_TO_MA)

#define TEST_EXPECTED_TEMP ((uint16_t)resistance_to_temp(voltage_to_res(TEST_TEMP_VOLTAGE_MV)))

#define TEST_MEASUREMENT_INTERVAL_MS 50
#define TEST_MEASUREMENT_INTERVAL_US ((TEST_MEASUREMENT_INTERVAL_MS)*1000)

#define TEST_GOOD_CELL_VOLTAGE_MV 3300
#define TEST_GOOD_CELL_VOLTAGE_SCALED_MV \
  (TEST_GOOD_CELL_VOLTAGE_MV * POWER_SELECT_CELL_VSENSE_SCALING / V_TO_MV)

#define TEST_LOW_CELL_VOLTAGE_MV 1900
#define TEST_LOW_CELL_VOLTAGE_SCALED_MV \
  (TEST_LOW_CELL_VOLTAGE_MV * POWER_SELECT_CELL_VSENSE_SCALING / V_TO_MV)

static void prv_force_measurement(void) {
  power_select_start(TEST_MEASUREMENT_INTERVAL_US);

  // Stop after one set of measurements taken
  delay_ms(TEST_MEASUREMENT_INTERVAL_MS - 10);
  power_select_stop();
}

// For comparing voltages/currents that get cast from float to uint16
#define TEST_FLOAT_CMP_DELTA_MAX 50

static CanStorage s_can_storage = { 0 };

static CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_POWER_SELECT,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = POWER_SELECT_CAN_EVENT_RX,
  .tx_event = POWER_SELECT_CAN_EVENT_TX,
  .fault_event = POWER_SELECT_CAN_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

static bool prv_gpio_addr_is_eq(GpioAddress addr0, GpioAddress addr1) {
  return (addr0.pin == addr1.pin) && (addr0.port == addr1.port);
}

// Set value returned on ADC read
// All power select measurements + cell pin reading
static uint16_t s_test_adc_read_values[NUM_POWER_SELECT_MEASUREMENTS + 1];

StatusCode TEST_MOCK(adc_read_converted_pin)(GpioAddress address, uint16_t *reading) {
  // Find correct reading to return
  for (uint8_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    if (prv_gpio_addr_is_eq(g_power_select_voltage_pins[i], address)) {
      *reading = s_test_adc_read_values[i];
      return STATUS_CODE_OK;
    }
  }
  for (uint8_t i = 0; i < NUM_POWER_SELECT_CURRENT_MEASUREMENTS; i++) {
    if (prv_gpio_addr_is_eq(g_power_select_current_pins[i], address)) {
      *reading = s_test_adc_read_values[i + NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS];
      return STATUS_CODE_OK;
    }
  }
  for (uint8_t i = 0; i < NUM_POWER_SELECT_TEMP_MEASUREMENTS; i++) {
    if (prv_gpio_addr_is_eq(g_power_select_temp_pins[i], address)) {
      *reading = s_test_adc_read_values[i + NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS +
                                        NUM_POWER_SELECT_CURRENT_MEASUREMENTS];
      return STATUS_CODE_OK;
    }
  }
  if (prv_gpio_addr_is_eq(g_power_select_cell_pin, address)) {
    *reading = s_test_adc_read_values[NUM_POWER_SELECT_MEASUREMENTS];
    return STATUS_CODE_OK;
  }

  // Should never get here
  return STATUS_CODE_INVALID_ARGS;
}

static GpioState s_test_gpio_read_states[NUM_POWER_SELECT_VALID_PINS + 1];

// Set value returned on GPIO read
// Generally used for valid pins
StatusCode TEST_MOCK(gpio_get_state)(GpioAddress *address, GpioState *input_state) {
  // Find correct reading to return
  for (uint8_t i = 0; i < NUM_POWER_SELECT_VALID_PINS; i++) {
    if (prv_gpio_addr_is_eq(g_power_select_valid_pins[i], *address)) {
      *input_state = s_test_gpio_read_states[i];
      return STATUS_CODE_OK;
    }
  }
  GpioAddress fault_addr = POWER_SELECT_DCDC_FAULT_ADDR;
  if (prv_gpio_addr_is_eq(fault_addr, *address)) {
    *input_state = s_test_gpio_read_states[NUM_POWER_SELECT_VALID_PINS];
    return STATUS_CODE_OK;
  }

  // Should never get here
  return STATUS_CODE_INVALID_ARGS;
}

static GpioState s_test_ltc_state;

// Since we mock out gpio_get_state for valid pins, we need to mock gpio_set_state
// to make sure calls to it work OK (only checking LTC)
StatusCode TEST_MOCK(gpio_set_state)(GpioAddress *address, GpioState state) {
  GpioAddress shdn_addr = POWER_SELECT_LTC_SHDN_ADDR;
  if (prv_gpio_addr_is_eq(shdn_addr, *address)) {
    s_test_ltc_state = state;
  }
  return STATUS_CODE_OK;
}

static void prv_set_voltages_good(void) {
  for (uint16_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    s_test_adc_read_values[i] = TEST_GOOD_VOLTAGE_SCALED_MV;
  }
  for (uint16_t i = 0; i < NUM_POWER_SELECT_CURRENT_MEASUREMENTS; i++) {
    s_test_adc_read_values[i + NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS] = TEST_GOOD_CURRENT_SCALED_MA;
  }
  for (uint16_t i = 0; i < NUM_POWER_SELECT_TEMP_MEASUREMENTS; i++) {
    s_test_adc_read_values[i + NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS +
                           NUM_POWER_SELECT_CURRENT_MEASUREMENTS] = TEST_TEMP_VOLTAGE_MV;
  }
  s_test_adc_read_values[NUM_POWER_SELECT_MEASUREMENTS] = TEST_GOOD_CELL_VOLTAGE_SCALED_MV;
}

static void prv_set_all_pins_valid(void) {
  for (uint16_t i = 0; i < NUM_POWER_SELECT_VALID_PINS; i++) {
    gpio_set_state(&g_power_select_valid_pins[i], GPIO_STATE_LOW);
  }
}

// For testing CAN broadcast
static uint16_t s_aux_measurements[4];

static StatusCode prv_power_select_aux_cb(const CanMessage *msg, void *context,
                                          CanAckStatus *ack_reply) {
  CAN_UNPACK_AUX_MEAS_MAIN_VOLTAGE(msg, &s_aux_measurements[0], &s_aux_measurements[1],
                                   &s_aux_measurements[2], &s_aux_measurements[3]);
  return STATUS_CODE_OK;
}

static uint16_t s_dcdc_measurements[4];

static StatusCode prv_power_select_dcdc_cb(const CanMessage *msg, void *context,
                                           CanAckStatus *ack_reply) {
  CAN_UNPACK_DCDC_MEAS_MAIN_CURRENT(msg, &s_dcdc_measurements[0], &s_dcdc_measurements[1],
                                    &s_dcdc_measurements[2], &s_dcdc_measurements[3]);
  return STATUS_CODE_OK;
}

static uint16_t s_status_readings[4];

static StatusCode prv_power_select_status_cb(const CanMessage *msg, void *context,
                                             CanAckStatus *ack_reply) {
  CAN_UNPACK_POWER_SELECT_STATUS(msg, &s_status_readings[0], &s_status_readings[1],
                                 &s_status_readings[2], &s_status_readings[3]);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  gpio_init();
  adc_init(ADC_MODE_SINGLE);
  gpio_it_init();
  can_init(&s_can_storage, &s_can_settings);

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_AUX_MEAS_MAIN_VOLTAGE, prv_power_select_aux_cb, NULL);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_DCDC_MEAS_MAIN_CURRENT, prv_power_select_dcdc_cb,
                          NULL);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_SELECT_STATUS, prv_power_select_status_cb, NULL);

  memset(s_aux_measurements, 0, sizeof(s_dcdc_measurements));
  memset(s_dcdc_measurements, 0, sizeof(s_dcdc_measurements));
  memset(s_status_readings, 0, sizeof(s_status_readings));
}

void teardown_test(void) {
  memset(s_test_adc_read_values, 0, NUM_POWER_SELECT_MEASUREMENTS * sizeof(uint16_t));

  // Valid pins are active-low
  memset(s_test_gpio_read_states, GPIO_STATE_LOW, NUM_POWER_SELECT_VALID_PINS * sizeof(GpioState));

  power_select_stop();
}

void test_power_select_init_works(void) {
  TEST_ASSERT_OK(power_select_init());

  // No faults, but pins invalid until we measure
  TEST_ASSERT_EQUAL(0, power_select_get_valid_bitset());
  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());

  // LTC should be activated
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_ltc_state);
}

void test_power_select_periodic_measure_works(void) {
  TEST_ASSERT_OK(power_select_init());

  TEST_ASSERT_OK(power_select_start(TEST_MEASUREMENT_INTERVAL_US));

  // Shouldn't be possible to start twice
  TEST_ASSERT_NOT_OK(power_select_start(TEST_MEASUREMENT_INTERVAL_US));

  // Make sure it doesn't break while running
  delay_ms(TEST_MEASUREMENT_INTERVAL_MS + 10);

  TEST_ASSERT_TRUE(power_select_stop());
  TEST_ASSERT_FALSE(power_select_stop());
}

void test_power_select_periodic_measure_reports_correctly(void) {
  TEST_ASSERT_OK(power_select_init());

  prv_set_voltages_good();
  prv_set_all_pins_valid();

  TEST_ASSERT_OK(power_select_start(TEST_MEASUREMENT_INTERVAL_US));

  delay_ms(TEST_MEASUREMENT_INTERVAL_MS + 10);

  // All pins should be valid, no faults
  TEST_ASSERT_EQUAL(0b111, power_select_get_valid_bitset());
  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());

  // LTC should be activated
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_ltc_state);

  // Check whether voltage readings as expected
  PowerSelectStorage test_storage = power_select_get_storage();

  for (uint16_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    // Account for float rounding
    TEST_ASSERT_UINT16_WITHIN(TEST_FLOAT_CMP_DELTA_MAX, TEST_GOOD_VOLTAGE_MV,
                              test_storage.voltages[i]);
    TEST_ASSERT_UINT16_WITHIN(TEST_FLOAT_CMP_DELTA_MAX, TEST_GOOD_CURRENT_MA,
                              test_storage.currents[i]);
    if (i < NUM_POWER_SELECT_TEMP_MEASUREMENTS) {
      // Confirm thermistor readings as expected
      TEST_ASSERT_INT32_WITHIN(TEST_FLOAT_CMP_DELTA_MAX, TEST_EXPECTED_TEMP, test_storage.temps[i]);
    }
  }
}

// Make sure that voltage/current at an invalid pin is read as 0
void test_power_select_invalid_pin_reading(void) {
  // Same as above initially
  TEST_ASSERT_OK(power_select_init());

  prv_set_voltages_good();
  prv_set_all_pins_valid();

  TEST_ASSERT_OK(power_select_start(TEST_MEASUREMENT_INTERVAL_US));

  delay_ms(TEST_MEASUREMENT_INTERVAL_MS + 10);

  // All pins should be valid, no faults
  TEST_ASSERT_EQUAL(0b111, power_select_get_valid_bitset());
  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());

  // Check whether voltage readings as expected
  PowerSelectStorage test_storage = power_select_get_storage();

  for (uint16_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    // Account for float rounding
    TEST_ASSERT_UINT16_WITHIN(TEST_FLOAT_CMP_DELTA_MAX, TEST_GOOD_VOLTAGE_MV,
                              test_storage.voltages[i]);
    TEST_ASSERT_UINT16_WITHIN(TEST_FLOAT_CMP_DELTA_MAX, TEST_GOOD_CURRENT_MA,
                              test_storage.currents[i]);
    if (i < NUM_POWER_SELECT_TEMP_MEASUREMENTS) {
      // Confirm thermistor readings as expected
      TEST_ASSERT_INT32_WITHIN(TEST_FLOAT_CMP_DELTA_MAX, TEST_EXPECTED_TEMP, test_storage.temps[i]);
    }
  }

  // Aux invalid
  s_test_gpio_read_states[POWER_SELECT_AUX] = GPIO_STATE_HIGH;

  delay_ms(TEST_MEASUREMENT_INTERVAL_MS + 10);

  // All pins should be valid except aux, no faults
  TEST_ASSERT_EQUAL(0b110, power_select_get_valid_bitset());
  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());

  // Check whether voltage readings as expected
  test_storage = power_select_get_storage();

  for (uint16_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    // Since aux invalid, both should be zero
    if (i == POWER_SELECT_AUX) {
      TEST_ASSERT_EQUAL(0, test_storage.voltages[i]);
      TEST_ASSERT_EQUAL(0, test_storage.currents[i]);
    } else {
      // Account for float rounding
      TEST_ASSERT_UINT16_WITHIN(TEST_FLOAT_CMP_DELTA_MAX, TEST_GOOD_VOLTAGE_MV,
                                test_storage.voltages[i]);
      TEST_ASSERT_UINT16_WITHIN(TEST_FLOAT_CMP_DELTA_MAX, TEST_GOOD_CURRENT_MA,
                                test_storage.currents[i]);
    }

    if (i < NUM_POWER_SELECT_TEMP_MEASUREMENTS) {
      // Confirm thermistor readings as expected
      TEST_ASSERT_INT32_WITHIN(TEST_FLOAT_CMP_DELTA_MAX, TEST_EXPECTED_TEMP, test_storage.temps[i]);
    }
  }
}

// Make sure fault bitset updated appropriately, faults broadcast over CAN correctly
// and LTC turned off on faults given in POWER_SELECT_LTC_DISABLE_FAULT_MASK
void test_power_select_faults_handled(void) {
  TEST_ASSERT_OK(power_select_init());

  prv_set_voltages_good();
  prv_set_all_pins_valid();

  TEST_ASSERT_OK(power_select_start(TEST_MEASUREMENT_INTERVAL_US));

  delay_ms(TEST_MEASUREMENT_INTERVAL_MS + 10);

  // All pins should be valid, no faults
  TEST_ASSERT_EQUAL(0b111, power_select_get_valid_bitset());
  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());

  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_ltc_state);

  // Set aux to overvoltage + overcurrent
  s_test_adc_read_values[POWER_SELECT_AUX] = TEST_FAULT_VOLTAGE_SCALED_MV;
  s_test_adc_read_values[POWER_SELECT_AUX + NUM_POWER_SELECT_MEASUREMENT_TYPES] =
      TEST_FAULT_CURRENT_SCALED_MA;
  delay_ms(TEST_MEASUREMENT_INTERVAL_MS + 10);

  uint16_t expected_fault_bitset = 0;
  expected_fault_bitset |= 1 << POWER_SELECT_FAULT_AUX_OV;
  expected_fault_bitset |= 1 << POWER_SELECT_FAULT_AUX_OC;
  TEST_ASSERT_EQUAL(expected_fault_bitset, power_select_get_fault_bitset());
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_test_ltc_state);

  // Set values back to good
  s_test_adc_read_values[POWER_SELECT_AUX] = TEST_GOOD_VOLTAGE_SCALED_MV;
  s_test_adc_read_values[POWER_SELECT_AUX + NUM_POWER_SELECT_MEASUREMENT_TYPES] =
      TEST_GOOD_CURRENT_SCALED_MA;
  delay_ms(TEST_MEASUREMENT_INTERVAL_MS + 10);

  // Should now be no faults
  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_ltc_state);
}

// Similar to above, but tests each individual fault on its own
void test_power_select_individual_fault_handling(void) {
  TEST_ASSERT_OK(power_select_init());

  prv_set_voltages_good();
  prv_set_all_pins_valid();

  TEST_ASSERT_OK(power_select_start(TEST_MEASUREMENT_INTERVAL_US));

  delay_ms(TEST_MEASUREMENT_INTERVAL_MS + 10);

  // All pins should be valid, no faults
  TEST_ASSERT_EQUAL(0b111, power_select_get_valid_bitset());
  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());

  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_ltc_state);

  for (uint8_t meas = 0; meas < NUM_POWER_SELECT_MEASUREMENT_TYPES; meas++) {
    LOG_DEBUG("Checking measurement %d\n", meas);

    // Overvoltage
    s_test_adc_read_values[meas] = TEST_FAULT_VOLTAGE_SCALED_MV;
    delay_ms(TEST_MEASUREMENT_INTERVAL_MS + 10);

    uint16_t expected_fault_bitset = 0;
    expected_fault_bitset |= 1 << meas;  // corresponding voltage fault
    TEST_ASSERT_EQUAL(expected_fault_bitset, power_select_get_fault_bitset());
    // Voltage faults don't turn off LTC
    TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_ltc_state);

    // Back to OK
    s_test_adc_read_values[meas] = TEST_GOOD_VOLTAGE_SCALED_MV;
    delay_ms(TEST_MEASUREMENT_INTERVAL_MS + 10);

    // Should now be no faults
    TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());
    TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_ltc_state);

    // Overcurrent
    s_test_adc_read_values[NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS + meas] =
        TEST_FAULT_CURRENT_SCALED_MA;
    delay_ms(TEST_MEASUREMENT_INTERVAL_MS + 10);

    expected_fault_bitset = 0;
    // Corresponding current fault
    expected_fault_bitset |= 1 << (NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS + meas);
    TEST_ASSERT_EQUAL(expected_fault_bitset, power_select_get_fault_bitset());

    // Only aux and pwr sup turn off LTC
    if ((meas == POWER_SELECT_AUX) || (meas == POWER_SELECT_PWR_SUP)) {
      TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_test_ltc_state);
    } else {
      TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_ltc_state);
    }

    // Back to OK
    s_test_adc_read_values[NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS + meas] =
        TEST_GOOD_CURRENT_SCALED_MA;
    delay_ms(TEST_MEASUREMENT_INTERVAL_MS + 10);

    // Should now be no faults
    TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());
    TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_ltc_state);
  }
}

// Make sure the DCDC fault pin works as expected
void test_power_select_dcdc_fault_works(void) {
  TEST_ASSERT_OK(power_select_init());

  prv_set_voltages_good();
  prv_set_all_pins_valid();

  prv_force_measurement();

  // All pins should be valid, no faults
  TEST_ASSERT_EQUAL(0b111, power_select_get_valid_bitset());
  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_ltc_state);

  // Trigger a DCDC fault interrupt
  s_test_gpio_read_states[NUM_POWER_SELECT_VALID_PINS] = GPIO_STATE_HIGH;
  GpioAddress pin = POWER_SELECT_DCDC_FAULT_ADDR;
  gpio_it_trigger_interrupt(&pin);

  TEST_ASSERT_EQUAL((1 << POWER_SELECT_FAULT_DCDC_PIN), power_select_get_fault_bitset());
  // DCDC faults don't turn off LTC
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_ltc_state);

  // Back to normal
  s_test_gpio_read_states[NUM_POWER_SELECT_VALID_PINS] = GPIO_STATE_LOW;
  gpio_it_trigger_interrupt(&pin);

  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_test_ltc_state);
}

// Make sure values get broadcast over CAN as expected
void test_power_select_broadcast_works(void) {
  TEST_ASSERT_OK(power_select_init());

  prv_set_voltages_good();
  prv_set_all_pins_valid();

  prv_force_measurement();

  Event e = { 0 };

  // 3 TX events, 3 RX events
  // We process events this way to make testing pass more consistently
  // locally + when running CI
  for (uint8_t i = 0; i < 6; i++) {
    MS_TEST_HELPER_AWAIT_EVENT(e);
    TEST_ASSERT_TRUE(can_process_event(&e));
  }

  // First TX: SYSTEM_CAN_MESSAGE_AUX_STATUS_MAIN_VOLTAGE
  TEST_ASSERT_EQUAL(TEST_GOOD_VOLTAGE_MV, s_aux_measurements[0]);  // aux voltage
  TEST_ASSERT_EQUAL(TEST_GOOD_CURRENT_MA, s_aux_measurements[1]);  // aux current
  TEST_ASSERT_EQUAL(TEST_EXPECTED_TEMP, s_aux_measurements[2]);    // aux temp
  TEST_ASSERT_EQUAL(TEST_GOOD_VOLTAGE_MV, s_aux_measurements[3]);  // main voltage

  // Second TX: SYSTEM_CAN_MESSAGE_DCDC_STATUS_MAIN_CURRENT
  TEST_ASSERT_EQUAL(TEST_GOOD_VOLTAGE_MV, s_dcdc_measurements[0]);  // dcdc voltage
  TEST_ASSERT_EQUAL(TEST_GOOD_CURRENT_MA, s_dcdc_measurements[1]);  // dcdc current
  TEST_ASSERT_EQUAL(TEST_EXPECTED_TEMP, s_dcdc_measurements[2]);    // dcdc temp
  TEST_ASSERT_EQUAL(TEST_GOOD_CURRENT_MA, s_dcdc_measurements[3]);  // main current

  // Third TX: SYSTEM_CAN_MESSAGE_POWER_SELECT_STATUS
  // Valid bitset should be 0b111, all others should be 0
  TEST_ASSERT_EQUAL(0, s_status_readings[0]);                          // Fault bitset
  TEST_ASSERT_EQUAL(0, s_status_readings[1]);                          // Warning bitset
  TEST_ASSERT_EQUAL(0b111, s_status_readings[2]);                      // Valid bitset
  TEST_ASSERT_EQUAL(TEST_GOOD_CELL_VOLTAGE_MV, s_status_readings[3]);  // Cell voltage

  // Should be no events
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Make sure POWER_SELECT_WARNING_BAT_LOW reported over CAN when battery < 2100 mV
void test_power_select_bat_low(void) {
  TEST_ASSERT_OK(power_select_init());

  prv_set_voltages_good();
  prv_set_all_pins_valid();

  prv_force_measurement();

  Event e = { 0 };

  // 3 TX events, 3 RX events
  for (uint8_t i = 0; i < 6; i++) {
    MS_TEST_HELPER_AWAIT_EVENT(e);
    TEST_ASSERT_TRUE(can_process_event(&e));
  }

  // Valid bitset should be 0b111, no faults/warnings
  TEST_ASSERT_EQUAL(0, s_status_readings[0]);                          // Fault bitset
  TEST_ASSERT_EQUAL(0, s_status_readings[1]);                          // Warning bitset
  TEST_ASSERT_EQUAL(0b111, s_status_readings[2]);                      // Valid bitset
  TEST_ASSERT_EQUAL(TEST_GOOD_CELL_VOLTAGE_MV, s_status_readings[3]);  // Cell voltage

  // Should be no events
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Now check low battery:
  s_test_adc_read_values[NUM_POWER_SELECT_MEASUREMENTS] = TEST_LOW_CELL_VOLTAGE_SCALED_MV;
  prv_force_measurement();

  // 3 TX events, 3 RX events
  for (uint8_t i = 0; i < 6; i++) {
    MS_TEST_HELPER_AWAIT_EVENT(e);
    TEST_ASSERT_TRUE(can_process_event(&e));
  }

  // Valid bitset should be 0b111, no faults, POWER_SELECT_WARNING_BAT_LOW bit set in warning
  TEST_ASSERT_EQUAL(0, s_status_readings[0]);                                    // Fault bitset
  TEST_ASSERT_EQUAL((1 << POWER_SELECT_WARNING_BAT_LOW), s_status_readings[1]);  // Warning bitset
  TEST_ASSERT_EQUAL(0b111, s_status_readings[2]);                                // Valid bitset
  TEST_ASSERT_EQUAL(TEST_LOW_CELL_VOLTAGE_MV, s_status_readings[3]);             // Cell voltage

  // Should be no events
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
