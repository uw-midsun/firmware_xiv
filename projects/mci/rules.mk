# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-common ms-helper


ifeq (x86,$(PLATFORM))
$(T)_EXCLUDE_TESTS := precharge
$(T)_test_drive_fsm_MOCKS := get_precharge_state
$(T)_test_cruise_rx_MOCKS := get_precharge_state
$(T)_test_mci_output_MOCKS := mcp2515_tx drive_fsm_get_drive_state drive_fsm_is_cruise
$(T)_test_mci_fan_control_MOCKS := gpio_get_state gpio_set_state
$(T)_test_mci_broadcast_MOCKS := mci_fan_get_fault_bitset
endif
