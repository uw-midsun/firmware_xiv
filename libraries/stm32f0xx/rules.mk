# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# This library glues the standard peripheral and CMSIS libraries together
# and includes the startup file.
$(T)_DEPS := STM32F0xx_StdPeriph_Driver version

# Specifies library specific build flags
$(T)_CFLAGS += -ffreestanding -nostdlib
