# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Redefine the source and include root folders
$(T)_SRC_ROOT := $($(T)_DIR)/Device/ST/STM32F0xx/Source/Templates
$(T)_INC_DIRS := $($(T)_DIR)/Device/ST/STM32F0xx/Include \
                 $($(T)_DIR)/Include

$(T)_DEPS :=

# Specifies library specific build flags
$(T)_CFLAGS += -ffreestanding -nostdlib
