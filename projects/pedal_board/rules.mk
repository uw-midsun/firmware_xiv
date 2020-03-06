# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-helper ms-common codegen-tooling

$(T)_test_brake_data_MOCKS := ads1015_read_raw

$(T)_test_throttle_data_MOCKS := ads1015_read_raw

$(T)_test_pedal_data_tx_MOCKS := ads1015_read_raw
