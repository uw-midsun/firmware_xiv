# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

$(T)_DEPS := $(PLATFORM_LIB) libcore
ifneq (,$(IS_MU))
$(T)_DEPS += mu-gen mu-store
endif

ifeq (x86,$(PLATFORM))
$(T)_EXCLUDE_TESTS := pwm pwm_input
$(T)_CFLAGS += -DX86
else 
$(T)_EXCLUDE_TESTS := wait
endif
