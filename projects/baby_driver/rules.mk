# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-common

$(T)_test_adc_read_MOCKS := adc_read_raw_pin adc_read_converted_pin
$(T)_test_gpio_get_MOCKS := gpio_get_state
$(T)_test_i2c_write_MOCKS := i2c_write i2c_write_reg
$(T)_test_gpio_interrupts_MOCKS := gpio_get_state
$(T)_test_spi_exchange_MOCKS := spi_exchange spi_init
