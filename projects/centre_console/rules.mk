# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-common ms-helper

$(T)_test_power_main_sequence_MOCKS := precharge_monitor_start \
																			 precharge_monitor_init \
																			 precharge_monitor_cancel

$(T)_test_drive_fsm_MOCKS := relay_tx_relay_state \
														 ebrake_tx_brake_state \
														 mci_output_tx_drive_output
