#include "power_select.h"
#include "power_select_events.h"
// #include "power_select_can.h" // give this its own separate test later!!!

#include "test_helpers.h"
#include "ms_test_helpers.h"
#include "log.h"
#include "gpio.h"
#include "interrupt.h"
#include "delay.h"
#include "string.h"
#include "thermistor.h"
#include "exported_enums.h"

#define TEST_CAN_ID 0x1

#define TEST_TEMP_VOLTAGE_MV 1100

#define TEST_GOOD_VOLTAGE_MV 3000
#define TEST_GOOD_CURRENT_MA 4000

// account for scaling 
#define TEST_GOOD_VOLTAGE_SCALED_MV (TEST_GOOD_VOLTAGE_MV * POWER_SELECT_VSENSE_SCALING / V_TO_MV)
#define TEST_GOOD_CURRENT_SCALED_MA (TEST_GOOD_CURRENT_MA * POWER_SELECT_ISENSE_SCALING / A_TO_MA)

#define TEST_FAULT_VOLTAGE_MV 20000
#define TEST_FAULT_CURRENT_MA 40000

// account for scaling
#define TEST_FAULT_VOLTAGE_SCALED_MV (TEST_FAULT_VOLTAGE_MV * POWER_SELECT_VSENSE_SCALING / V_TO_MV)
#define TEST_FAULT_CURRENT_SCALED_MA (TEST_FAULT_CURRENT_MA * POWER_SELECT_ISENSE_SCALING / A_TO_MA)

static void prv_force_measurement(void) {
  power_select_start();
  
  // wait for measurement to finish
  delay_ms(20);
  power_select_stop();
}

static CanStorage s_can_storage = { 0 };

static CanSettings s_can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_POWER_SELECTION,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = POWER_SELECT_CAN_EVENT_RX,
    .tx_event = POWER_SELECT_CAN_EVENT_TX,
    .fault_event = POWER_SELECT_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
};

// Pin defs, copy-pasted from power_select.c
static const GpioAddress VOLTAGE_MEASUREMENT_PINS[NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_VSENSE_ADDR,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_VSENSE_ADDR,
  [POWER_SELECT_PWR_SUP] = POWER_SELECT_PWR_SUP_VSENSE_ADDR,
};

static const GpioAddress CURRENT_MEASUREMENT_PINS[NUM_POWER_SELECT_CURRENT_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_ISENSE_ADDR,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_ISENSE_ADDR,
  [POWER_SELECT_PWR_SUP] = POWER_SELECT_PWR_SUP_ISENSE_ADDR,
};

static const GpioAddress TEMP_MEASUREMENT_PINS[NUM_POWER_SELECT_TEMP_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_TEMP_ADDR,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_TEMP_ADDR,
};

static const uint16_t MAX_VOLTAGES[NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_MAX_VOLTAGE_MV,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_MAX_VOLTAGE_MV,
  [POWER_SELECT_PWR_SUP] = POWER_SELECT_PWR_SUP_MAX_VOLTAGE_MV,
};

static const uint16_t MAX_CURRENTS[NUM_POWER_SELECT_CURRENT_MEASUREMENTS] = {
  [POWER_SELECT_AUX] = POWER_SELECT_AUX_MAX_CURRENT_MA,
  [POWER_SELECT_DCDC] = POWER_SELECT_DCDC_MAX_CURRENT_MA,
  [POWER_SELECT_PWR_SUP] = POWER_SELECT_PWR_SUP_MAX_CURRENT_MA,
};

static const GpioAddress VALID_PINS[NUM_POWER_SELECT_VALID_PINS] = {
  POWER_SELECT_AUX_VALID_ADDR,
  POWER_SELECT_DCDC_VALID_ADDR,
  POWER_SELECT_PWR_SUP_VALID_ADDR,
};

static bool prv_gpio_addr_is_eq(GpioAddress addr0, GpioAddress addr1) {
  return (addr0.pin == addr1.pin) && (addr0.port == addr1.port);
}

// set value returned on ADC read
static uint16_t s_test_adc_read_values[NUM_POWER_SELECT_MEASUREMENTS];

StatusCode TEST_MOCK(adc_read_converted_pin)(GpioAddress address, uint16_t *reading) {
    // Find correct reading to return
    for(int i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
      if(prv_gpio_addr_is_eq(VOLTAGE_MEASUREMENT_PINS[i], address)) {
        *reading = s_test_adc_read_values[i];
        return STATUS_CODE_OK;    
      }
    }
    for(int i = 0; i < NUM_POWER_SELECT_CURRENT_MEASUREMENTS; i++) {
      if(prv_gpio_addr_is_eq(CURRENT_MEASUREMENT_PINS[i], address)) {
        *reading = s_test_adc_read_values[i + NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS];
        return STATUS_CODE_OK;    
      }
    }
    for(int i = 0; i < NUM_POWER_SELECT_TEMP_MEASUREMENTS; i++) {
      if(prv_gpio_addr_is_eq(TEMP_MEASUREMENT_PINS[i], address)) {
        *reading = s_test_adc_read_values[i + NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS + NUM_POWER_SELECT_CURRENT_MEASUREMENTS];
        return STATUS_CODE_OK;    
      }
    }
  
  // should never get here
  return STATUS_CODE_INVALID_ARGS;
}

// set value returned on GPIO read
static GpioState s_test_gpio_read_states[NUM_POWER_SELECT_VALID_PINS];
static uint8_t s_test_gpio_read_index = 0;

StatusCode TEST_MOCK(gpio_get_state)(GpioAddress *address, GpioState *input_state) {
    *input_state = s_test_gpio_read_states[s_test_gpio_read_index];
    
    if(s_test_gpio_read_index == NUM_POWER_SELECT_VALID_PINS - 1) {
      s_test_gpio_read_index = 0;
    } else {
      s_test_gpio_read_index++;
    }

    return STATUS_CODE_OK;
}

static void prv_set_voltages_good(void) {
  for(uint16_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    s_test_adc_read_values[i] = TEST_GOOD_VOLTAGE_SCALED_MV;
  }
  s_test_adc_read_values[3] = TEST_GOOD_CURRENT_SCALED_MA;
  s_test_adc_read_values[5] = TEST_GOOD_CURRENT_SCALED_MA;
  s_test_adc_read_values[4] = TEST_GOOD_CURRENT_SCALED_MA;
  s_test_adc_read_values[6] = TEST_TEMP_VOLTAGE_MV;
  s_test_adc_read_values[7] = TEST_TEMP_VOLTAGE_MV;
}

static void prv_set_all_pins_valid(void) {
  for(uint16_t i = 0; i < NUM_POWER_SELECT_VALID_PINS; i++) {
    s_test_gpio_read_states[i] = GPIO_STATE_LOW;
  }
}

// For testing CAN broadcast
static uint16_t s_ab_mv_measurements[4];

static StatusCode prv_power_select_ab_mv_cb(const CanMessage *msg, void *context,
                                         CanAckStatus *ack_reply) {  
  CAN_UNPACK_AUX_BATTERY_STATUS_MAIN_POWER_VOLTAGE(msg, &s_ab_mv_measurements[0], &s_ab_mv_measurements[1], &s_ab_mv_measurements[2], &s_ab_mv_measurements[3]);
  return STATUS_CODE_OK;
}

static uint16_t s_db_mc_measurements[4];

static StatusCode prv_power_select_db_mc_cb(const CanMessage *msg, void *context,
                                         CanAckStatus *ack_reply) {  
  CAN_UNPACK_DCDC_BATTERY_STATUS_MAIN_POWER_CURRENT(msg, &s_db_mc_measurements[0], &s_db_mc_measurements[1], &s_db_mc_measurements[2], &s_db_mc_measurements[3]);
  return STATUS_CODE_OK;
}

static uint64_t s_fault_measurement;

static StatusCode prv_power_select_fault_cb(const CanMessage *msg, void *context,
                                         CanAckStatus *ack_reply) {  
  CAN_UNPACK_POWER_SELECT_FAULT(msg, &s_fault_measurement);
  return STATUS_CODE_OK;
}


void setup_test(void) {
    gpio_init();
    interrupt_init();
    soft_timer_init();
    adc_init(ADC_MODE_SINGLE);
    event_queue_init();
    can_init(&s_can_storage, &s_can_settings);
}

void teardown_test(void) {
  memset(s_test_adc_read_values, 0, NUM_POWER_SELECT_MEASUREMENTS * sizeof(uint16_t));

  // valid pins are active-low
  memset(s_test_gpio_read_states, GPIO_STATE_LOW, NUM_POWER_SELECT_VALID_PINS * sizeof(GpioState));

  power_select_stop();

  s_test_gpio_read_index = 0;
}

void test_power_select_init_works(void) {
    TEST_ASSERT_OK(power_select_init());
}

void test_power_select_periodic_measure_works(void) {
    TEST_ASSERT_OK(power_select_init());

    TEST_ASSERT_OK(power_select_start());

    // Make sure it doesn't break while running
    delay_ms(POWER_SELECT_MEASUREMENT_INTERVAL_MS * 2); 

    power_select_stop();
}

void test_power_select_periodic_measure_reports_correctly(void) {
  TEST_ASSERT_OK(power_select_init());
  // TEST_ASSERT_OK(power_select_can_init());

  prv_set_voltages_good();
  prv_set_all_pins_valid();
  
  TEST_ASSERT_OK(power_select_start());

  delay_ms(POWER_SELECT_MEASUREMENT_INTERVAL_MS * 2);

  // all pins should be valid, no faults
  TEST_ASSERT_EQUAL(0b111, power_select_get_valid_bitset());
  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());

  // check whether voltage readings as expected
  PowerSelectStorage test_storage = power_select_get_storage();


  uint16_t expected_temp = (uint16_t)resistance_to_temp(voltage_to_res(TEST_TEMP_VOLTAGE_MV));
  for(uint16_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    // account for float rounding
    TEST_ASSERT_TRUE(abs(test_storage.voltages[i] - TEST_GOOD_VOLTAGE_MV) < 50);
    TEST_ASSERT_TRUE(abs(test_storage.currents[i] - TEST_GOOD_CURRENT_MA) < 50);
    if(i < NUM_POWER_SELECT_TEMP_MEASUREMENTS) {
      // confirm thermistor readings as expected
      TEST_ASSERT_TRUE(abs(test_storage.temps[i] - expected_temp) < 50);
    }
  }
}

// Make sure that voltage/current at an invalid pin is read as 0
void test_power_select_invalid_pin_reading(void) {
  // Same as above initially
   TEST_ASSERT_OK(power_select_init());

  prv_set_voltages_good();
  prv_set_all_pins_valid();
  
  TEST_ASSERT_OK(power_select_start());

  delay_ms(POWER_SELECT_MEASUREMENT_INTERVAL_MS * 2);

  // all pins should be valid, no faults
  TEST_ASSERT_EQUAL(0b111, power_select_get_valid_bitset());
  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());

  // check whether voltage readings as expected
  PowerSelectStorage test_storage = power_select_get_storage();


  uint16_t expected_temp = (uint16_t)resistance_to_temp(voltage_to_res(TEST_TEMP_VOLTAGE_MV));
  for(uint16_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    // account for float rounding
    TEST_ASSERT_TRUE(abs(test_storage.voltages[i] - TEST_GOOD_VOLTAGE_MV) < 50);
    TEST_ASSERT_TRUE(abs(test_storage.currents[i] - TEST_GOOD_CURRENT_MA) < 50);
    if(i < NUM_POWER_SELECT_TEMP_MEASUREMENTS) {
      // confirm thermistor readings as expected
      TEST_ASSERT_TRUE(abs(test_storage.temps[i] - expected_temp) < 50);
    }
  }

  // Aux invalid
  s_test_gpio_read_states[POWER_SELECT_AUX] = GPIO_STATE_HIGH;

  delay_ms(POWER_SELECT_MEASUREMENT_INTERVAL_MS * 2);

  // all pins should be valid except aux, no faults
  TEST_ASSERT_EQUAL(0b110, power_select_get_valid_bitset());
  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());

  // check whether voltage readings as expected
  test_storage = power_select_get_storage();

  expected_temp = (uint16_t)resistance_to_temp(voltage_to_res(TEST_TEMP_VOLTAGE_MV));
  for(uint16_t i = 0; i < NUM_POWER_SELECT_VOLTAGE_MEASUREMENTS; i++) {
    // since aux invalid, both should be zero
    if(i == POWER_SELECT_AUX) {
      TEST_ASSERT_EQUAL(0, test_storage.voltages[i]);
      TEST_ASSERT_EQUAL(0, test_storage.currents[i]);
    } else {
      // account for float rounding
      TEST_ASSERT_TRUE(abs(test_storage.voltages[i] - TEST_GOOD_VOLTAGE_MV) < 50);
      TEST_ASSERT_TRUE(abs(test_storage.currents[i] - TEST_GOOD_CURRENT_MA) < 50);
    }

    if(i < NUM_POWER_SELECT_TEMP_MEASUREMENTS) {
      // confirm thermistor readings as expected
      TEST_ASSERT_TRUE(abs(test_storage.temps[i] - expected_temp) < 50);
    }
  }
}

// Make sure fault bitset updated appropriately, faults broadcast over CAN correctly
void test_power_select_faults_handled(void) {
    TEST_ASSERT_OK(power_select_init());

  prv_set_voltages_good();
  prv_set_all_pins_valid();
  
  TEST_ASSERT_OK(power_select_start());

  delay_ms(POWER_SELECT_MEASUREMENT_INTERVAL_MS * 2);

  // all pins should be valid, no faults
  TEST_ASSERT_EQUAL(0b111, power_select_get_valid_bitset());
  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());

  // set aux to overvoltage + overcurrent
  s_test_adc_read_values[POWER_SELECT_AUX] = TEST_FAULT_VOLTAGE_SCALED_MV;
  s_test_adc_read_values[POWER_SELECT_AUX + NUM_POWER_SELECT_MEASUREMENT_TYPES] = TEST_FAULT_CURRENT_SCALED_MA;
  delay_ms(POWER_SELECT_MEASUREMENT_INTERVAL_MS * 2);
  
  uint16_t expected_fault_bitset = 0;
  expected_fault_bitset |= 1 << POWER_SELECT_AUX_OVERVOLTAGE;
  expected_fault_bitset |= 1 << POWER_SELECT_AUX_OVERCURRENT;
  TEST_ASSERT_EQUAL(expected_fault_bitset, power_select_get_fault_bitset());

  // set values back to good
  s_test_adc_read_values[POWER_SELECT_AUX] = TEST_GOOD_VOLTAGE_SCALED_MV;
  s_test_adc_read_values[POWER_SELECT_AUX + NUM_POWER_SELECT_MEASUREMENT_TYPES] = TEST_GOOD_CURRENT_SCALED_MA;
  delay_ms(POWER_SELECT_MEASUREMENT_INTERVAL_MS * 2);

  // should now be no faults
  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());
}


// Make sure the DCDC fault pin works as expected
void test_power_select_dcdc_fault_works(void) {
  TEST_ASSERT_OK(power_select_init());

  prv_set_voltages_good();
  prv_set_all_pins_valid();

  prv_force_measurement();

  // all pins should be valid, no faults
  TEST_ASSERT_EQUAL(0b111, power_select_get_valid_bitset());
  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());

  // Trigger a DCDC fault interrupt
  s_test_gpio_read_states[0] = GPIO_STATE_HIGH;
  GpioAddress pin = POWER_SELECT_DCDC_FAULT_ADDR;
  gpio_it_trigger_interrupt(&pin); // doesn't work

  TEST_ASSERT_EQUAL((1 << POWER_SELECT_DCDC_FAULT), power_select_get_fault_bitset());

  // Back to normal
  s_test_gpio_read_states[0] = GPIO_STATE_LOW;
  gpio_it_trigger_interrupt(&pin);

  TEST_ASSERT_EQUAL(0, power_select_get_fault_bitset());
}

// Make sure values get broadcast over CAN as expected
void test_power_select_broadcast_works(void) {
  TEST_ASSERT_OK(power_select_init());

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_AUX_BATTERY_STATUS, prv_power_select_ab_mv_cb, NULL);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_DCDC_BATTERY_STATUS, prv_power_select_db_mc_cb, NULL);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_SELECT_FAULT, prv_power_select_fault_cb, NULL);

  memset(s_ab_mv_measurements, 0, sizeof(s_db_mc_measurements));
  memset(s_db_mc_measurements, 0, sizeof(s_db_mc_measurements));
  s_fault_measurement = 0;
  
  prv_set_voltages_good();
  prv_set_all_pins_valid();

  prv_force_measurement();

  // 3 total broadcasts from measurement (2x measurements + fault)
  MS_TEST_HELPER_CAN_TX(POWER_SELECT_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_TX(POWER_SELECT_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_TX(POWER_SELECT_CAN_EVENT_TX);

   // first rx should be SYSTEM_CAN_MESSAGE_AUX_BATTERY_STATUS
  MS_TEST_HELPER_CAN_RX(POWER_SELECT_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(TEST_GOOD_VOLTAGE_MV, s_ab_mv_measurements[0]); // aux voltage
  TEST_ASSERT_EQUAL(TEST_GOOD_CURRENT_MA, s_ab_mv_measurements[1]); // aux current
  TEST_ASSERT_EQUAL(TEST_GOOD_VOLTAGE_MV, s_ab_mv_measurements[3]); // main voltage

  // second rx should be SYSTEM_CAN_MESSAGE_DCDC_BATTERY_STATUS
  MS_TEST_HELPER_CAN_RX(POWER_SELECT_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(TEST_GOOD_VOLTAGE_MV, s_db_mc_measurements[0]); // dcdc voltage
  TEST_ASSERT_EQUAL(TEST_GOOD_CURRENT_MA, s_db_mc_measurements[1]); // dcdc current
  TEST_ASSERT_EQUAL(TEST_GOOD_CURRENT_MA, s_db_mc_measurements[3]); // main current
  
  // third rx should be SYSTEM_CAN_MESSAGE_POWER_SELECT_FAULT
  MS_TEST_HELPER_CAN_RX(POWER_SELECT_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(0, s_fault_measurement);
  
  // should be no events
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
