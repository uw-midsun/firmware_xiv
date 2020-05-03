# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the device library you want to include
$(T)_DEPS := ms-common ms-helper

$(T)_INC_DIRS += $($(T)_DIR)/config

ifeq (x86,$(PLATFORM))
$(T)_EXCLUDE_TESTS := ltc_afe ltc_adc
endif

$(T)_test_bps_heartbeat_MOCKS := sequenced_relay_set_state
