# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-common codegen-tooling

$(T)_test_power_select_MOCKS := adc_read_converted_pin gpio_get_state gpio_set_state
$(T)_test_power_select_can_MOCKS := power_select_get_fault_bitset power_select_get_valid_bitset
