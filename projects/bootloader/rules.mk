# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
$(T)_SRC_ROOT:= $($(T)_DIR)
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_SRC := $(wildcard $($(T)_SRC_ROOT)/src/*.c) \
            $(wildcard $($(T)_SRC_ROOT)/src/$(PLATFORM)/*.c) \
            $(wildcard $($(T)_SRC_ROOT)/src/*.s) \
			$(wildcard $($(T)_SRC_ROOT)/protogen/*.c)

$(T)_INC_DIRS += $($(T)_DIR)/protogen

$(T)_DEPS := ms-common ms-helper nanopb

$(T)_LINKER_SCRIPT := stm32f0_bootloader.ld
$(T)_test_config_MOCKS := persist_init
