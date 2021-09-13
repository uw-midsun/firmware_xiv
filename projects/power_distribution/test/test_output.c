#include "output.h"

#include <stdbool.h>
#include <stdint.h>

#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "output_config.h"
#include "pca9539r_gpio_expander.h"
#include "pin_defs.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_ADC_READING 1000

#define TEST_OUTPUT 3
#define TEST_OUTPUT_2 5

#define TEST_MUX_SEL 5

#define TEST_ASSERT_PCA9539R_STATE(address, expected_state)             \
  ({                                                                    \
    Pca9539rGpioState actual_state = NUM_PCA9539R_GPIO_STATES;          \
    TEST_ASSERT_OK(pca9539r_gpio_get_state(&(address), &actual_state)); \
    TEST_ASSERT_EQUAL((expected_state), actual_state);                  \
  })

#define TEST_ASSERT_EQUAL_GPIO_ADDRESS(expected, actual)                                        \
  ({                                                                                            \
    TEST_ASSERT_EQUAL_MESSAGE((expected).port, (actual).port, "GpioAddress ports don't match"); \
    TEST_ASSERT_EQUAL_MESSAGE((expected).pin, (actual).pin, "GpioAddress pins don't match");    \
  })

static GpioAddress s_test_mux_output_pin = PD_MUX_OUTPUT_PIN;

// for BTS7200
static Pca9539rGpioAddress s_test_dsel_pin = { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_0 };
static Pca9539rGpioAddress s_test_en_pin_0 = { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_1 };
static Pca9539rGpioAddress s_test_en_pin_1 = { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_2 };

// for BTS7040
static Pca9539rGpioAddress s_test_en_pin = { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_2 };

// for GPIO
static GpioAddress s_test_gpio_pin = { GPIO_PORT_B, 2 };

static GpioAddress s_adc_read_address;
static size_t s_times_adc_read = 0;

StatusCode TEST_MOCK(adc_read_converted_pin)(GpioAddress address, uint16_t *reading) {
  s_times_adc_read++;
  s_adc_read_address = address;
  *reading = TEST_ADC_READING;
  return STATUS_CODE_OK;
}

static const GpioAddress *s_gpio_set_address;
static GpioState s_set_gpio_state;

StatusCode TEST_MOCK(gpio_set_state)(const GpioAddress *address, GpioState state) {
  s_gpio_set_address = address;
  s_set_gpio_state = state;
  return STATUS_CODE_OK;
}

static const I2CAddress s_i2c_addrs[] = { PD_PCA9539R_I2C_ADDRESS_0, PD_PCA9539R_I2C_ADDRESS_1 };

static void prv_set_config_boilerplate(OutputConfig *config) {
  config->i2c_port = PD_I2C_PORT;
  config->i2c_addresses = s_i2c_addrs;
  config->num_i2c_addresses = SIZEOF_ARRAY(s_i2c_addrs);
  config->mux_address = (MuxAddress){
    .bit_width = 4,
    .sel_pins =
        {
            PD_MUX_SEL1_PIN,
            PD_MUX_SEL2_PIN,
            PD_MUX_SEL3_PIN,
            PD_MUX_SEL4_PIN,
        },
  };
  config->mux_enable_pin = (GpioAddress)PD_MUX_ENABLE_PIN;
  config->mux_output_pin = s_test_mux_output_pin;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  adc_init(ADC_MODE_SINGLE);

  I2CSettings settings = {
    .speed = I2C_SPEED_FAST,
    .scl = PD_I2C_SCL_PIN,
    .sda = PD_I2C_SDA_PIN,
  };
  i2c_init(PD_I2C_PORT, &settings);

  s_times_adc_read = 0;
  s_gpio_set_address = NULL;
  s_set_gpio_state = NUM_GPIO_STATES;
}

void teardown_test(void) {}

// Test that BTS7200 outputs with the same OutputBts7200Info use the same storage.
void test_output_bts7200_storage_combined(void) {
  // generate some valid OutputBts7200Infos to use
  OutputBts7200Info bts7200_info[3];
  for (size_t i = 0; i < SIZEOF_ARRAY(bts7200_info); i++) {
    bts7200_info[i].dsel_pin = s_test_dsel_pin;
    bts7200_info[i].enable_0_pin = s_test_en_pin_0;
    bts7200_info[i].enable_1_pin = s_test_en_pin_1;
    bts7200_info[i].mux_selection = TEST_MUX_SEL;
  }
  // for my sanity I have to disable clang-format's inane formatting for this struct
  // clang-format off
  OutputConfig config = {
    .specs = {
      [0] = {
        .type = OUTPUT_TYPE_BTS7200,
        .on_front = true,
        .bts7200_spec = {
          .bts7200_info = &bts7200_info[0],
          .channel = 0,
        },
      },
      [1] = {
        .type = OUTPUT_TYPE_BTS7200,
        .on_front = true,
        .bts7200_spec = {
          .bts7200_info = &bts7200_info[1],
          .channel = 0,
        },
      },
      [2] = {
        .type = OUTPUT_TYPE_BTS7200,
        .on_front = true,
        .bts7200_spec = {
          .bts7200_info = &bts7200_info[2],
          .channel = 0,
        },
      },
      [3] = {
        .type = OUTPUT_TYPE_BTS7200,
        .on_front = true,
        .bts7200_spec = {
          .bts7200_info = &bts7200_info[0],
          .channel = 1,
        },
      },
      [4] = {
        .type = OUTPUT_TYPE_BTS7200,
        .on_front = true,
        .bts7200_spec = {
          .bts7200_info = &bts7200_info[1],
          .channel = 1,
        },
      },
      [5] = {
        .type = OUTPUT_TYPE_BTS7200,
        .on_front = true,
        .bts7200_spec = {
          .bts7200_info = &bts7200_info[2],
          .channel = 3,
        },
      },
    },
  };
  // clang-format on
  prv_set_config_boilerplate(&config);
  TEST_ASSERT_OK(output_init(&config, true));

  // each of the outputs with the same bts7200 info have the same storage
  TEST_ASSERT_EQUAL_PTR(test_output_get_bts7200_storage(0), test_output_get_bts7200_storage(3));
  TEST_ASSERT_EQUAL_PTR(test_output_get_bts7200_storage(1), test_output_get_bts7200_storage(4));
  TEST_ASSERT_EQUAL_PTR(test_output_get_bts7200_storage(2), test_output_get_bts7200_storage(5));
}

// Test that output_set_state works for BTS7200 outputs.
void test_output_set_state_bts7200(void) {
  OutputBts7200Info bts7200_info = {
    .dsel_pin = s_test_dsel_pin,
    .enable_0_pin = s_test_en_pin_0,
    .enable_1_pin = s_test_en_pin_1,
    .mux_selection = TEST_MUX_SEL,
  };
  OutputConfig config = {
    .specs =
        {
            [TEST_OUTPUT] =
                {
                    .type = OUTPUT_TYPE_BTS7200,
                    .on_front = true,
                    .bts7200_spec =
                        {
                            .bts7200_info = &bts7200_info,
                            .channel = 0,
                        },
                },
            [TEST_OUTPUT_2] =
                {
                    .type = OUTPUT_TYPE_BTS7200,
                    .on_front = true,
                    .bts7200_spec =
                        {
                            .bts7200_info = &bts7200_info,
                            .channel = 1,
                        },
                },
        },
  };
  prv_set_config_boilerplate(&config);
  TEST_ASSERT_OK(output_init(&config, true));

  // make sure they're using the same storage since they're on the same OutputBts7200Info
  Bts7200Storage *storage_1 = test_output_get_bts7200_storage(TEST_OUTPUT);
  Bts7200Storage *storage_2 = test_output_get_bts7200_storage(TEST_OUTPUT_2);
  TEST_ASSERT_EQUAL_PTR(storage_1, storage_2);

  TEST_ASSERT_OK(output_set_state(TEST_OUTPUT, OUTPUT_STATE_ON));
  TEST_ASSERT_PCA9539R_STATE(s_test_en_pin_0, PCA9539R_GPIO_STATE_HIGH);

  TEST_ASSERT_OK(output_set_state(TEST_OUTPUT_2, OUTPUT_STATE_ON));
  TEST_ASSERT_PCA9539R_STATE(s_test_en_pin_1, PCA9539R_GPIO_STATE_HIGH);

  TEST_ASSERT_OK(output_set_state(TEST_OUTPUT, OUTPUT_STATE_OFF));
  TEST_ASSERT_PCA9539R_STATE(s_test_en_pin_0, PCA9539R_GPIO_STATE_LOW);

  TEST_ASSERT_OK(output_set_state(TEST_OUTPUT_2, OUTPUT_STATE_OFF));
  TEST_ASSERT_PCA9539R_STATE(s_test_en_pin_1, PCA9539R_GPIO_STATE_LOW);
}

// Test that output_read_current works for BTS7200 outputs.
// Note that we don't test the actual value read since it's based on hardware-dependent parameters.
void test_output_read_current_bts7200(void) {
  OutputBts7200Info bts7200_info = {
    .dsel_pin = s_test_dsel_pin,
    .enable_0_pin = s_test_en_pin_0,
    .enable_1_pin = s_test_en_pin_1,
    .mux_selection = TEST_MUX_SEL,
  };
  OutputConfig config = {
    .specs =
        {
            [TEST_OUTPUT] =
                {
                    .type = OUTPUT_TYPE_BTS7200,
                    .on_front = true,
                    .bts7200_spec =
                        {
                            .bts7200_info = &bts7200_info,
                            .channel = 0,
                        },
                },
            [TEST_OUTPUT_2] =
                {
                    .type = OUTPUT_TYPE_BTS7200,
                    .on_front = true,
                    .bts7200_spec =
                        {
                            .bts7200_info = &bts7200_info,
                            .channel = 1,
                        },
                },
        },
  };
  prv_set_config_boilerplate(&config);
  TEST_ASSERT_OK(output_init(&config, true));

  // first, dsel pin must go low to read from channel 0
  uint16_t current = 0;
  TEST_ASSERT_OK(output_read_current(TEST_OUTPUT, &current));
  TEST_ASSERT_PCA9539R_STATE(s_test_dsel_pin, PCA9539R_GPIO_STATE_LOW);
  TEST_ASSERT_EQUAL(1, s_times_adc_read);
  TEST_ASSERT_EQUAL_GPIO_ADDRESS(s_test_mux_output_pin, s_adc_read_address);
  TEST_ASSERT_NOT_EQUAL(0, current);

  // then it goes high when we read from channel 1
  current = 0;
  TEST_ASSERT_OK(output_read_current(TEST_OUTPUT_2, &current));
  TEST_ASSERT_PCA9539R_STATE(s_test_dsel_pin, PCA9539R_GPIO_STATE_HIGH);
  TEST_ASSERT_EQUAL(2, s_times_adc_read);
  TEST_ASSERT_EQUAL_GPIO_ADDRESS(s_test_mux_output_pin, s_adc_read_address);
  TEST_ASSERT_NOT_EQUAL(0, current);

  // and back to low when we read from channel 0 again
  current = 0;
  TEST_ASSERT_OK(output_read_current(TEST_OUTPUT, &current));
  TEST_ASSERT_PCA9539R_STATE(s_test_dsel_pin, PCA9539R_GPIO_STATE_LOW);
  TEST_ASSERT_EQUAL(3, s_times_adc_read);
  TEST_ASSERT_EQUAL_GPIO_ADDRESS(s_test_mux_output_pin, s_adc_read_address);
  TEST_ASSERT_NOT_EQUAL(0, current);
}

// Test that output_set_state works for a BTS7040 output.
void test_output_set_state_bts7040(void) {
  OutputConfig config = {
    .specs =
        {
            [TEST_OUTPUT] =
                {
                    .type = OUTPUT_TYPE_BTS7040,
                    .on_front = true,
                    .bts7040_spec =
                        {
                            .enable_pin = s_test_en_pin,
                            .mux_selection = TEST_MUX_SEL,
                        },
                },
        },
  };
  prv_set_config_boilerplate(&config);
  TEST_ASSERT_OK(output_init(&config, true));

  TEST_ASSERT_OK(output_set_state(TEST_OUTPUT, OUTPUT_STATE_ON));
  TEST_ASSERT_PCA9539R_STATE(s_test_en_pin, PCA9539R_GPIO_STATE_HIGH);

  TEST_ASSERT_OK(output_set_state(TEST_OUTPUT, OUTPUT_STATE_OFF));
  TEST_ASSERT_PCA9539R_STATE(s_test_en_pin, PCA9539R_GPIO_STATE_LOW);

  TEST_ASSERT_OK(output_set_state(TEST_OUTPUT, OUTPUT_STATE_ON));
  TEST_ASSERT_PCA9539R_STATE(s_test_en_pin, PCA9539R_GPIO_STATE_HIGH);
}

// Test that output_read_current works for a BTS7040 output.
// Again we don't test the actual value read because it's based on hardware-dependent parameters.
void test_output_read_current_bts7040(void) {
  OutputConfig config = {
    .specs =
        {
            [TEST_OUTPUT] =
                {
                    .type = OUTPUT_TYPE_BTS7040,
                    .on_front = false,
                    .bts7040_spec =
                        {
                            .enable_pin = s_test_en_pin,
                            .mux_selection = TEST_MUX_SEL,
                        },
                },
        },
  };
  prv_set_config_boilerplate(&config);
  TEST_ASSERT_OK(output_init(&config, false));

  uint16_t current;
  TEST_ASSERT_OK(output_read_current(TEST_OUTPUT, &current));
  TEST_ASSERT_EQUAL(1, s_times_adc_read);
  TEST_ASSERT_EQUAL_GPIO_ADDRESS(s_test_mux_output_pin, s_adc_read_address);
  TEST_ASSERT_NOT_EQUAL(0, current);
}

// check that read current with the bts7004 flag has different value than normal bts7040
void test_output_read_current_bts7004(void) {
  OutputConfig bts7004config = {
    .specs =
        {
            [TEST_OUTPUT] =
                {
                    .type = OUTPUT_TYPE_BTS7040,
                    .on_front = false,
                    .bts7040_spec =
                        {
                            .enable_pin = s_test_en_pin,
                            .mux_selection = TEST_MUX_SEL,
                            .use_bts7004_scaling = true,
                        },
                },
        },
  };
  prv_set_config_boilerplate(&bts7004config);
  TEST_ASSERT_OK(output_init(&bts7004config, false));

  uint16_t bts7004current;
  TEST_ASSERT_OK(output_read_current(TEST_OUTPUT, &bts7004current));
  TEST_ASSERT_NOT_EQUAL(0, bts7004current);

  OutputConfig bts7040config = {
    .specs =
        {
            [TEST_OUTPUT] =
                {
                    .type = OUTPUT_TYPE_BTS7040,
                    .on_front = false,
                    .bts7040_spec =
                        {
                            .enable_pin = s_test_en_pin,
                            .mux_selection = TEST_MUX_SEL,
                            .use_bts7004_scaling = true,
                        },
                },
        },
  };

  prv_set_config_boilerplate(&bts7040config);
  TEST_ASSERT_OK(output_init(&bts7040config, false));

  uint16_t bts7040current;
  TEST_ASSERT_OK(output_read_current(TEST_OUTPUT, &bts7040current));
  TEST_ASSERT_EQUAL(2, s_times_adc_read);
  TEST_ASSERT_EQUAL_GPIO_ADDRESS(s_test_mux_output_pin, s_adc_read_address);
  TEST_ASSERT_NOT_EQUAL(bts7004current, bts7040current);
}

// Test that output_set_state works for a GPIO output.
void test_output_set_state_gpio(void) {
  OutputConfig config = {
    .specs =
        {
            [TEST_OUTPUT] =
                {
                    .type = OUTPUT_TYPE_GPIO,
                    .on_front = true,
                    .gpio_spec =
                        {
                            .address = s_test_gpio_pin,
                        },
                },
        },
  };
  prv_set_config_boilerplate(&config);
  TEST_ASSERT_OK(output_init(&config, true));

  TEST_ASSERT_OK(output_set_state(TEST_OUTPUT, OUTPUT_STATE_ON));
  TEST_ASSERT_NOT_NULL(s_gpio_set_address);
  TEST_ASSERT_EQUAL_GPIO_ADDRESS(s_test_gpio_pin, *s_gpio_set_address);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_set_gpio_state);

  TEST_ASSERT_OK(output_set_state(TEST_OUTPUT, OUTPUT_STATE_OFF));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_set_gpio_state);

  TEST_ASSERT_OK(output_set_state(TEST_OUTPUT, OUTPUT_STATE_ON));
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_set_gpio_state);
}

// Test that we can't read current from a GPIO output.
void test_output_read_current_gpio_doesnt_work(void) {
  OutputConfig config = {
    .specs =
        {
            [TEST_OUTPUT] =
                {
                    .type = OUTPUT_TYPE_GPIO,
                    .on_front = true,
                    .gpio_spec =
                        {
                            .address = s_test_gpio_pin,
                        },
                },
        },
  };
  prv_set_config_boilerplate(&config);
  TEST_ASSERT_OK(output_init(&config, true));
  uint16_t current;
  TEST_ASSERT_NOT_OK(output_read_current(TEST_OUTPUT, &current));
}

// Test that the real output config initializes correctly.
TEST_CASE(true)
TEST_CASE(false)
void test_init_with_real_config(bool is_front) {
  TEST_ASSERT_OK(output_init(&g_combined_output_config, is_front));
}

// Test that we fail gracefully with invalid config to output_init.
void test_invalid_config(void) {
  TEST_ASSERT_NOT_OK(output_init(NULL, false));

  OutputConfig invalid_config = { 0 };
  prv_set_config_boilerplate(&invalid_config);
  // normally ok with no outputs specified
  TEST_ASSERT_OK(output_init(&invalid_config, false));

  // invalid mux pins
  GpioAddress invalid_address = { NUM_GPIO_PORTS, 0 };
  invalid_config.mux_enable_pin = invalid_address;
  TEST_ASSERT_NOT_OK(output_init(&invalid_config, false));
  invalid_config.mux_enable_pin = (GpioAddress)PD_MUX_ENABLE_PIN;
  invalid_config.mux_address.sel_pins[0] = invalid_address;
  TEST_ASSERT_NOT_OK(output_init(&invalid_config, false));
  invalid_config.mux_address.sel_pins[0] = (GpioAddress)PD_MUX_SEL1_PIN;

  // GPIO: invalid address
  invalid_config.specs[TEST_OUTPUT] = (OutputSpec){
    .type = OUTPUT_TYPE_GPIO,
    .on_front = false,
    .gpio_spec =
        {
            .address = { NUM_GPIO_PORTS, 99 },
        },
  };
  TEST_ASSERT_NOT_OK(output_init(&invalid_config, false));

  // BTS7200: null bts7200 info
  invalid_config.specs[TEST_OUTPUT] = (OutputSpec){
    .type = OUTPUT_TYPE_BTS7200,
    .on_front = false,
    .bts7200_spec =
        {
            .channel = 0,
            .bts7200_info = NULL,
        },
  };
  TEST_ASSERT_NOT_OK(output_init(&invalid_config, false));

  // invalid pin in bts7200 info
  Pca9539rGpioAddress invalid_pca9539r_pin = { PD_PCA9539R_I2C_ADDRESS_0, NUM_PCA9539R_GPIO_PINS };
  OutputBts7200Info invalid_bts7200_info = {
    .enable_0_pin = invalid_pca9539r_pin,
    .enable_1_pin = invalid_pca9539r_pin,
    .dsel_pin = invalid_pca9539r_pin,
    .mux_selection = 0,
  };
  invalid_config.specs[TEST_OUTPUT].bts7200_spec.bts7200_info = &invalid_bts7200_info;
  TEST_ASSERT_NOT_OK(output_init(&invalid_config, false));

  // BTS7040: invalid pin
  invalid_config.specs[TEST_OUTPUT] = (OutputSpec){
    .type = OUTPUT_TYPE_BTS7200,
    .on_front = false,
    .bts7040_spec =
        {
            .enable_pin = invalid_pca9539r_pin,
            .mux_selection = 0,
        },
  };
  TEST_ASSERT_NOT_OK(output_init(&invalid_config, false));
}

// Test that the various output functions reject an invalid output.
void test_invalid_output(void) {
  OutputConfig config = { 0 };
  prv_set_config_boilerplate(&config);
  TEST_ASSERT_OK(output_init(&config, false));

  TEST_ASSERT_NOT_OK(output_set_state(NUM_OUTPUTS, OUTPUT_STATE_OFF));
  uint16_t current;
  TEST_ASSERT_NOT_OK(output_read_current(NUM_OUTPUTS, &current));
}
