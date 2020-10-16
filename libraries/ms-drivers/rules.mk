# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-common


ifeq (x86,$(PLATFORM))
$(T)_test_ltc_afe_MOCKS := spi_exchange
$(T)_test_ads1259_adc_MOCKS := spi_exchange
$(T)_test_adt7476a_fan_controller_MOCKS := i2c_write i2c_read_reg
$(T)_test_bts_7200_load_switch_MOCKS := adc_read_converted
$(T)_test_bts_7040_load_switch_MOCKS := adc_read_converted
endif
