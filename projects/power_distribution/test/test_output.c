#include "output.h"

#include <stdbool.h>

#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "pca9539r_gpio_expander.h"
#include "pin_defs.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_OUTPUT 3
#define TEST_OUTPUT_2 5

#define TEST_MUX_SEL 5

#define TEST_ASSERT_PCA9539R_STATE(address, expected_state)             \
  ({                                                                    \
    Pca9539rGpioState actual_state = NUM_PCA9539R_GPIO_STATES;          \
    TEST_ASSERT_OK(pca9539r_gpio_get_state(&(address), &actual_state)); \
    TEST_ASSERT_EQUAL((expected_state), actual_state);                  \
  })

static Pca9539rGpioAddress s_test_dsel_pin = { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_0 };
static Pca9539rGpioAddress s_test_en_pin_0 = { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_1 };
static Pca9539rGpioAddress s_test_en_pin_1 = { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_1 };

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
  config->mux_output_pin = (GpioAddress)PD_MUX_OUTPUT_PIN;
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
  OutputConfig config = {
    .specs =
        {
            [0] =
                {
                    .type = OUTPUT_TYPE_BTS7200,
                    .on_front = true,
                    .bts7200_spec =
                        {
                            .bts7200_info = &bts7200_info[0],
                            .channel = 0,
                        },
                },
            [1] =
                {
                    .type = OUTPUT_TYPE_BTS7200,
                    .on_front = true,
                    .bts7200_spec =
                        {
                            .bts7200_info = &bts7200_info[1],
                            .channel = 0,
                        },
                },
            [2] =
                {
                    .type = OUTPUT_TYPE_BTS7200,
                    .on_front = true,
                    .bts7200_spec =
                        {
                            .bts7200_info = &bts7200_info[2],
                            .channel = 0,
                        },
                },
            [3] =
                {
                    .type = OUTPUT_TYPE_BTS7200,
                    .on_front = true,
                    .bts7200_spec =
                        {
                            .bts7200_info = &bts7200_info[0],
                            .channel = 1,
                        },
                },
            [4] =
                {
                    .type = OUTPUT_TYPE_BTS7200,
                    .on_front = true,
                    .bts7200_spec =
                        {
                            .bts7200_info = &bts7200_info[1],
                            .channel = 1,
                        },
                },
            [5] =
                {
                    .type = OUTPUT_TYPE_BTS7200,
                    .on_front = true,
                    .bts7200_spec =
                        {
                            .bts7200_info = &bts7200_info[2],
                            .channel = 3,
                        },
                },
        },
  };
  prv_set_config_boilerplate(&config);
  TEST_ASSERT_OK(output_init(&config, true));

  // each of the outputs with the same bts7200 info have the same storage
  TEST_ASSERT_EQUAL_PTR(test_output_get_bts7200_storage(0), test_output_get_bts7200_storage(3));
  TEST_ASSERT_EQUAL_PTR(test_output_get_bts7200_storage(1), test_output_get_bts7200_storage(4));
  TEST_ASSERT_EQUAL_PTR(test_output_get_bts7200_storage(2), test_output_get_bts7200_storage(5));
}

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
