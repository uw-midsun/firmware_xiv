# Specify toolchain
CC := $(GCC_ARM_BASE)arm-none-eabi-gcc
LD := $(GCC_ARM_BASE)arm-none-eabi-gcc
OBJCPY := $(GCC_ARM_BASE)arm-none-eabi-objcopy
OBJDUMP := $(GCC_ARM_BASE)arm-none-eabi-objdump
SIZE := $(GCC_ARM_BASE)arm-none-eabi-size
AR := $(GCC_ARM_BASE)arm-none-eabi-gcc-ar
GDB := $(GCC_ARM_BASE)arm-none-eabi-gdb
OPENOCD := openocd

# Set the library to include if using this platform
PLATFORM_LIB := stm32f0xx
PLATFORM_EXT := .elf

# Architecture dependent variables
ARCH_CFLAGS := -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb

# Linker script location
LDSCRIPT_DIR := $(PLATFORM_DIR)/ldscripts

# Helper scripts location
SCRIPT_DIR := $(PLATFORM_DIR)/scripts

# Build flags for the device
CDEFINES := USE_STDPERIPH_DRIVER STM32F072 HSE_VALUE=32000000
CFLAGS := -Wall -Wextra -Werror -g3 -Os -std=c11 -Wno-discarded-qualifiers \
					-Wno-unused-variable -Wno-unused-parameter -Wsign-conversion -Wpointer-arith \
					-ffunction-sections -fdata-sections \
					$(ARCH_CFLAGS) $(addprefix -D,$(CDEFINES))

# Linker flags
LDFLAGS := -L$(LDSCRIPT_DIR) -Tstm32f0.ld -Wl,--gc-sections -Wl,--undefined=uxTopUsedPriority \
           --specs=nosys.specs --specs=nano.specs

# Device openocd config file
# Use PROBE=stlink-v2 for discovery boards
PROBE=cmsis-dap
OPENOCD_SCRIPT_DIR := /usr/share/openocd/scripts/
OPENOCD_CFG := -s $(OPENOCD_SCRIPT_DIR) \
               -f interface/$(PROBE).cfg \
               -f target/stm32f0x.cfg \
               -c "$$(python3 $(SCRIPT_DIR)/select_programmer.py $(SERIAL))" \
               -f $(SCRIPT_DIR)/stm32f0-openocd.cfg \
               -c 'stm32f0x.cpu configure -rtos FreeRTOS'

# Platform targets
.PHONY: program gdb target

ifeq (,$(MACOS_SSH_USERNAME))

program: $(TARGET_BINARY:$(PLATFORM_EXT)=.bin)
	@$(OPENOCD) $(OPENOCD_CFG) -c "stm_flash $<" -c shutdown

gdb: $(TARGET_BINARY)
	@pkill $(OPENOCD) || true
	@setsid $(OPENOCD) $(OPENOCD_CFG) > /dev/null 2>&1 &
	@$(GDB) $< -x "$(SCRIPT_DIR)/gdb_flash"
	@pkill $(OPENOCD)

define session_wrapper
pkill $(OPENOCD) || true
setsid $(OPENOCD) $(OPENOCD_CFG) > /dev/null 2>&1 &
$1; pkill $(OPENOCD)
endef

# Defines command to run for unit testing
define test_run
clear && $(GDB) $1 -x "$(SCRIPT_DIR)/gdb_flash" -ex "b LoopForever" -ex "c" -ex "set confirm off" -ex "q"
endef

else

# VirtualBox default NAT IP
MACOS_SSH_IP := 10.0.2.2
MAKE_ARGS := TEST PROJECT LIBRARY PLATFORM PROBE SERIAL
MAKE_PARAMS := $(foreach arg,$(MAKE_ARGS),$(arg)=$($(arg)))
SSH_CMD := ssh -t $(MACOS_SSH_USERNAME)@$(MACOS_SSH_IP) "bash -c 'cd $(MACOS_SSH_BOX_PATH)/shared/firmware && make $(MAKECMDGOALS) $(MAKE_PARAMS)'"

.PHONY: unsupported run_ssh

# Use additional dependencies - hopefully they run first, otherwise we'll build unnecessarily
# Unfortunately this only works part of the time
test_all: unsupported
test: run_ssh

# Host is macOS so we can't pass the programmer through - do it all through SSH
program gdb:
	@echo "Running command through SSH"
	@$(SSH_CMD)

ifneq (,$(TEST))
run_ssh:
	@echo "Running command through SSH"
	@$(SSH_CMD:make=make -o $(TARGET_BINARY))
else
run_ssh: unsupported
endif

unsupported:
	@echo "Please specify a single test to run."
	@echo "Consecutive tests for STM32F0xx are not supported on a macOS host." && false

endif
