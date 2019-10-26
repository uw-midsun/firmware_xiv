# Specify toolchain
COMPILER := gcc
VALID_COMPILERS := gcc clang
override COMPILER := $(filter $(VALID_COMPILERS),$(COMPILER))
ifeq (,$(COMPILER))
  $(error Invalid compiler. Expected: $(VALID_COMPILERS))
endif

CC := $(COMPILER)
LD := $(COMPILER)
OBJCPY := objcopy
OBJDUMP := objdump
SIZE := size
AR := ar
GDB := gdb

# Set the library to include if using this platform
PLATFORM_LIB := x86
PLATFORM_EXT :=

# Architecture dependent variables
ARCH_CFLAGS :=

# Linker and startup script locations
LDSCRIPT := $(PLATFORM_DIR)/ldscripts

# Build flags for the device
CDEFINES := _GNU_SOURCE

ifeq (gcc,$(COMPILER))
  CSFLAGS := -g -Os
else ifeq (asan, $(COPTIONS))
  CSFLAGS := -O1 -g -fsanitize=address -fno-omit-frame-pointer
else ifeq (tsan, $(COPTIONS))
  CSFLAGS := -fsanitize=thread -O1 -g
else
  CSFLAGS := -O1 -g
endif

CFLAGS := $(CSFLAGS) -Wall -Wextra -Werror -std=gnu11 -Wno-discarded-qualifiers \
          -Wno-unused-variable -Wno-unused-parameter -Wsign-conversion -Wpointer-arith \
          -ffunction-sections -fdata-sections -pthread \
          $(ARCH_CFLAGS) $(addprefix -D,$(CDEFINES))

ifeq (clang,$(COMPILER))
  CFLAGS := $(filter-out -Wno-discarded-qualifiers,$(CFLAGS)) -Wno-missing-field-initializers \
            -Wno-incompatible-pointer-types-discards-qualifiers -Wno-missing-braces
endif

# Linker flags
LDFLAGS := -lrt

# Shell environment variables
FLASH_VAR := MIDSUN_X86_FLASH_FILE
ifneq (,$(filter test test_all,$(MAKECMDGOALS)))
ifeq (,$(TEST))
  ENV_VARS = $(FLASH_VAR)=$(test)_flash
else
  ENV_VARS = $(FLASH_VAR)=$<_flash
endif
else
  ENV_VARS := $(FLASH_VAR)=$(BIN_DIR)/$(PROJECT)$(LIBRARY)_flash
endif

# Platform targets
.PHONY: run gdb

run: $(BIN_DIR)/$(PROJECT)$(PLATFORM_EXT) socketcan
	@$(ENV_VARS) $<

gdb: $(TARGET_BINARY) socketcan
	@$(ENV_VARS) $(GDB) $<

test_all: socketcan

test: socketcan

define session_wrapper
$(ENV_VARS) $1
endef

# Defines command to run for unit testing
define test_run
echo "\nRunning $(notdir $1)" && $(ENV_VARS) ./$1
endef
