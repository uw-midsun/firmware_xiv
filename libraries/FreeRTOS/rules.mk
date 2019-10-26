# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# The code in this library is pulled directly from the FreeRTOS release.
# https://www.freertos.org/
#
# Any Midnight Sun specific changes should go in the ms-freertos library

$(T)_SRC_ROOT := $($(T)_DIR)

$(T)_INC_DIRS := $($(T)_DIR) \
                  $($(T)_DIR)/include

$(T)_SRC := $(wildcard $($(T)_SRC_ROOT)/*.c) \
            $(wildcard $($(T)_SRC_ROOT)/*.s)

# TODO: Integrate the Linux simulator
ifeq (stm32f0xx,$(PLATFORM))
    $(T)_INC_DIRS += $($(T)_DIR)/portable/GCC/ARM_CM0
    $(T)_SRC += $(wildcard $($(T)_SRC_ROOT)/portable/GCC/ARM_CM0/*.c)
else ifeq (x86,$(PLATFORM))
    $(T)_INC_DIRS += $($(T)_DIR)/portable/GCC/Linux
    $(T)_SRC += $(wildcard $($(T)_SRC_ROOT)/portable/GCC/Linux/*.c)
endif

ifeq (stm32f0xx,$(PLATFORM))
$(T)_DEPS := CMSIS stm32f0xx ms-freertos
else ifeq (x86,$(PLATFORM))
$(T)_DEPS := ms-freertos
endif

# Specifies library specific build flags
$(T)_CFLAGS += -ffreestanding -nostdlib
