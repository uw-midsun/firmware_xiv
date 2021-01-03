#include <stdbool.h>
#include <string.h>

#include "adc.h"
#include "adt7476a_fan_controller.h"
#include "adt7476a_fan_controller_defs.h"
#include "can.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "delay.h"
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

#define TEST_I2C_PORT I2C_PORT_2
#define TEST_I2C_ADDRESS 0x1

#define TEST_CONFIG_PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define TEST_CONFIG_PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

#define TEST_PWM_1 ADT_PWM_PORT_1
#define TEST_PWM_2 ADT_PWM_PORT_2

#define ADT7476A_PWM_1 0x30
#define ADT7476A_PWM_3 0x32

#define OVERTEMP_FLAGS (FAN_OVERTEMP_TRIGGERED | DCDC_OVERTEMP | ENCLOSURE_OVERTEMP)
#define OVERVOLT_FLAGS (VCC_EXCEEDED | VCCP_EXCEEDED)
#define FAN_ERR_FLAGS (FAN1_STATUS | FAN2_STATUS | FAN3_STATUS | FAN4_STATUS)

#define ADC_MAX_VAL 3300

#define FAN_UNDERTEMP_VOLTAGE 1163
#define FAN_OVERTEMP_VOLTAGE 758
#define FAN_50_PERC_VOLTAGE 975

#define FAN_OVERTEMP_FRACTION_TRANSMIT (FAN_OVERTEMP_VOLTAGE * 1000) / ADC_MAX_VAL

#define FAN_MAX_I2C_WRITE 0xFF  // equates to (percent value)/0.39 -> adt7476A conversion
#define FAN_50_PERC_I2C_WRITE 0x7F
#define FAN_MIN_I2C_WRITE 0x0

static uint16_t s_fan_ctrl_msg[4];
static CanStorage s_can_storage;

static FanCtrlSettings s_fan_settings = {
  .i2c_port = TEST_I2C_PORT,
  .fan_pwm1 = TEST_PWM_1,
  .fan_pwm2 = TEST_PWM_2,
  .i2c_address = TEST_I2C_ADDRESS,
};

static uint16_t adc_ret_val;
StatusCode TEST_MOCK(adc_read_converted_pin)(GpioAddress address, uint16_t *reading) {
  *reading = adc_ret_val;
  return STATUS_CODE_OK;
}

static uint8_t i2c_buf1[10];
static uint8_t i2c_buf2[10];
StatusCode TEST_MOCK(i2c_write)(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len) {
  // Save both temp values written over i2c
  if (tx_data[0] == ADT7476A_PWM_1) {
    memcpy(i2c_buf1, tx_data, tx_len);
  } else {
    memcpy(i2c_buf2, tx_data, tx_len);
  }
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(i2c_read_reg)(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data,
                                   size_t rx_len) {
  if (reg == ADT7476A_INTERRUPT_STATUS_REGISTER_1) {
    *rx_data = OVERVOLT_FLAGS;
  } else if (reg == ADT7476A_INTERRUPT_STATUS_REGISTER_2) {
    *rx_data = FAN_ERR_FLAGS;
  }
  return STATUS_CODE_OK;
}

static StatusCode prv_front_can_fan_ctrl_rx_handler(const CanMessage *msg, void *context,
                                                    CanAckStatus *ack_reply) {
  CAN_UNPACK_FRONT_FAN_FAULT(msg, &s_fan_ctrl_msg[0]);
  return STATUS_CODE_OK;
}

static StatusCode prv_rear_can_fan_ctrl_rx_handler(const CanMessage *msg, void *context,
                                                   CanAckStatus *ack_reply) {
  CAN_UNPACK_REAR_FAN_FAULT(msg, &s_fan_ctrl_msg[0], &s_fan_ctrl_msg[1], &s_fan_ctrl_msg[2],
                            &s_fan_ctrl_msg[3]);
  return STATUS_CODE_OK;
}

static void prv_initialize_can(SystemCanDevice can_device) {
  CanSettings can_settings = {
    .device_id = can_device,
    .loopback = true,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = POWER_DISTRIBUTION_CAN_EVENT_RX,
    .tx_event = POWER_DISTRIBUTION_CAN_EVENT_TX,
    .fault_event = POWER_DISTRIBUTION_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  can_init(&s_can_storage, &can_settings);
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  adc_init(ADC_MODE_SINGLE);
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = TEST_CONFIG_PIN_I2C_SCL,
    .sda = TEST_CONFIG_PIN_I2C_SDA,
  };
  i2c_init(TEST_I2C_PORT, &i2c_settings);
  gpio_it_init();
  memset(s_fan_ctrl_msg, 0, 4 * sizeof(uint16_t));
}

void teardown_test(void) {}

void test_fan_ctrl_init(void) {
  TEST_ASSERT_NOT_OK(pd_fan_ctrl_init(NULL, true));
  TEST_ASSERT_OK(pd_fan_ctrl_init(&s_fan_settings, true));
  TEST_ASSERT_OK(pd_fan_ctrl_init(&s_fan_settings, false));
}

// Test Gpio interrupt on rear smbalert pin triggered
void test_fan_err_rear(void) {
  prv_initialize_can(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR);
  TEST_ASSERT_OK(pd_fan_ctrl_init(&s_fan_settings, false));
  gpio_it_trigger_interrupt(&(GpioAddress)PD_SMBALERT_PIN);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_REAR_FAN_FAULT, prv_rear_can_fan_ctrl_rx_handler,
                          NULL);
  MS_TEST_HELPER_CAN_TX_RX(POWER_DISTRIBUTION_CAN_EVENT_TX, POWER_DISTRIBUTION_CAN_EVENT_RX);
  TEST_ASSERT_TRUE(((s_fan_ctrl_msg[0] >> 8) & OVERVOLT_FLAGS) == OVERVOLT_FLAGS);
  TEST_ASSERT_TRUE((s_fan_ctrl_msg[0] & FAN_ERR_FLAGS) == FAN_ERR_FLAGS);
}

// Test Gpio interrupt on front smbalert pin triggered
void test_fan_err_front(void) {
  prv_initialize_can(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT);
  TEST_ASSERT_OK(pd_fan_ctrl_init(&s_fan_settings, true));
  gpio_it_trigger_interrupt(&(GpioAddress)PD_SMBALERT_PIN);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_FRONT_FAN_FAULT, prv_front_can_fan_ctrl_rx_handler,
                          NULL);
  MS_TEST_HELPER_CAN_TX_RX(POWER_DISTRIBUTION_CAN_EVENT_TX, POWER_DISTRIBUTION_CAN_EVENT_RX);
  TEST_ASSERT_TRUE((((s_fan_ctrl_msg[0]) >> 8) & OVERVOLT_FLAGS) == OVERVOLT_FLAGS);
  TEST_ASSERT_TRUE((s_fan_ctrl_msg[0] & FAN_ERR_FLAGS) == FAN_ERR_FLAGS);
}

void test_rear_pd_fan_ctrl_temp(void) {
  // Check overtemp, undertemp, and 50% temp values transmit correct
  // values and execute properly

  // set adc to return temp that converts to 50% speed
  adc_ret_val = FAN_50_PERC_VOLTAGE;
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, pd_fan_ctrl_init(&s_fan_settings, false));
  delay_ms(REAR_FAN_CONTROL_REFRESH_PERIOD_MILLIS + 10);
  TEST_ASSERT_EQUAL(FAN_50_PERC_I2C_WRITE, i2c_buf1[1]);
  TEST_ASSERT_EQUAL(FAN_50_PERC_I2C_WRITE, i2c_buf2[1]);

  // set adc to return undertemp
  adc_ret_val = FAN_UNDERTEMP_VOLTAGE;
  delay_ms(REAR_FAN_CONTROL_REFRESH_PERIOD_MILLIS);
  TEST_ASSERT_EQUAL(FAN_MIN_I2C_WRITE, i2c_buf1[1]);
  TEST_ASSERT_EQUAL(FAN_MIN_I2C_WRITE, i2c_buf2[1]);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Test adc returns an overtemp value - CAN message with overtemp values transmitted
  adc_ret_val = FAN_OVERTEMP_VOLTAGE;
  prv_initialize_can(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR);
  delay_ms(REAR_FAN_CONTROL_REFRESH_PERIOD_MILLIS);
  TEST_ASSERT_EQUAL(FAN_MAX_I2C_WRITE, i2c_buf1[1]);
  TEST_ASSERT_EQUAL(FAN_MAX_I2C_WRITE, i2c_buf2[1]);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_REAR_FAN_FAULT, prv_rear_can_fan_ctrl_rx_handler,
                          NULL);
  MS_TEST_HELPER_CAN_TX_RX(POWER_DISTRIBUTION_CAN_EVENT_TX, POWER_DISTRIBUTION_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(ADC_MAX_VAL, s_fan_ctrl_msg[3]);
  TEST_ASSERT_EQUAL(s_fan_ctrl_msg[1], s_fan_ctrl_msg[2]);
  TEST_ASSERT_EQUAL(s_fan_ctrl_msg[1],
                    FAN_OVERTEMP_FRACTION_TRANSMIT);  // FAN_OVERTEMP_VOLTAGE as a fraction of v_ref
  // Check Overtemp byte set correctly
  TEST_ASSERT_EQUAL(OVERTEMP_FLAGS, ((s_fan_ctrl_msg[0] >> 8) & OVERTEMP_FLAGS));
}

void test_front_pd_fan_ctrl_pot(void) {
  // Check max, min, and 50% potentiometer values transmit correct
  // values and execute properly

  // set adc to return max value
  adc_ret_val = ADC_MAX_VAL;
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, pd_fan_ctrl_init(&s_fan_settings, true));
  delay_ms(REAR_FAN_CONTROL_REFRESH_PERIOD_MILLIS + 10);
  TEST_ASSERT_EQUAL(FAN_MAX_I2C_WRITE, i2c_buf1[1]);
  TEST_ASSERT_EQUAL(FAN_MAX_I2C_WRITE, i2c_buf2[1]);

  // set adc to return mid value
  adc_ret_val = ADC_MAX_VAL / 2;
  delay_ms(REAR_FAN_CONTROL_REFRESH_PERIOD_MILLIS);
  TEST_ASSERT_EQUAL(FAN_50_PERC_I2C_WRITE, i2c_buf1[1]);
  TEST_ASSERT_EQUAL(FAN_50_PERC_I2C_WRITE, i2c_buf2[1]);

  // set adc to return min value
  adc_ret_val = 0;
  delay_ms(REAR_FAN_CONTROL_REFRESH_PERIOD_MILLIS);
  TEST_ASSERT_EQUAL(FAN_MIN_I2C_WRITE, i2c_buf1[1]);
  TEST_ASSERT_EQUAL(FAN_MIN_I2C_WRITE, i2c_buf2[1]);
}
