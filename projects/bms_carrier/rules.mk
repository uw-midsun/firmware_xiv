# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-common ms-helper ms-drivers

ifeq (x86,$(PLATFORM))
$(T)_test_killswitch_MOCKS := gpio_get_state fault_bps
$(T)_test_bps_heartbeat_MOCKS := fault_bps
$(T)_test_cell_sense_MOCKS := current_sense_is_charging ltc_afe_process_event
endif
# Uses mocked fault handling to verify internal logic
# TODO(SOFT-61): Should allow modules to hook into internal fault state (or read can messages) so this isn't needed
$(T)_test_cell_sense_MOCKS += fault_bps
