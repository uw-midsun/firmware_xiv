# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-common ms-helper

ifeq (x86,$(PLATFORM))
$(T)_test_control_pilot_MOCKS := pwm_input_get_reading
$(T)_test_connection_sense_MOCKS := adc_read_converted
$(T)_test_charger_controller_MOCKS := generic_can_tx mcp2515_register_cbs
$(T)_test_begin_sequence_MOCKS := charger_controller_activate gpio_set_state gpio_get_state
$(T)_test_stop_sequence_MOCKS := charger_controller_deactivate gpio_set_state
endif
