# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-common ms-helper ms-drivers

$(T)_test_main_event_generator_MOCKS := get_pedal_state \
                                        drive_fsm_get_global_state \
                                        power_fsm_get_current_state \
                                        get_global_speed_state \
                                        get_global_charging_state

$(T)_test_race_switch_MOCKS := gpio_get_state
