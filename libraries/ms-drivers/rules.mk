# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-common
ifneq (,$(IS_MU))
$(T)_DEPS += mu-gen mu-store
endif

ifeq (x86,$(PLATFORM))
$(T)_test_ltc_afe_MOCKS := spi_exchange
$(T)_test_ads1259_adc_MOCKS := spi_exchange
$(T)_test_adt7476a_fan_controller_MOCKS := i2c_write i2c_read_reg
endif

$(T)_test_bts7200_load_switch_MOCKS := adc_read_converted_pin
$(T)_test_bts7040_load_switch_MOCKS := adc_read_converted_pin
$(T)_test_voltage_regulator_MOCKS := gpio_get_state
$(T)_test_generic_gpio_MOCKS := gpio_set_state gpio_get_state pca9539r_gpio_set_state pca9539r_gpio_get_state
