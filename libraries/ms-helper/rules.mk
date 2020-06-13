# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-common

ifeq (x86,$(PLATFORM))
$(T)_EXCLUDE_TESTS := mcp2515 adc_periodic_reader
endif

$(T)_test_thermistor_MOCKS := adc_read_converted adc_get_channel adc_set_channel
