# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-common ms-drivers

$(T)_test_mppt_MOCKS := mux_set

$(T)_test_sense_MOCKS := data_store_done
$(T)_test_sense_current_MOCKS := data_store_set sense_register mcp3427_register_callback \
	mcp3427_register_fault_callback mcp3427_start
$(T)_test_sense_mppt_MOCKS := sense_register mppt_read_current mppt_read_voltage_in mppt_read_pwm \
	mppt_read_status
$(T)_test_sense_temperature_MOCKS := sense_register adc_read_raw
