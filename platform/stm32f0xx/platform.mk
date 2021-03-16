# Helper scripts location
SCRIPT_DIR := $(PLATFORM_DIR)/scripts

# Specify toolchain
CC := $(GCC_ARM_BASE)arm-none-eabi-gcc
LD := $(GCC_ARM_BASE)arm-none-eabi-gcc
OBJCPY := $(GCC_ARM_BASE)arm-none-eabi-objcopy
OBJDUMP := $(GCC_ARM_BASE)arm-none-eabi-objdump
SIZE := bash $(SCRIPT_DIR)/pretty_size.sh $(GCC_ARM_BASE)arm-none-eabi-size
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

# Build flags for the device
CDEFINES := USE_STDPERIPH_DRIVER STM32F072 HSE_VALUE=32000000
CFLAGS := -Wall -Wextra -Werror -g3 -Os -std=c11 -Wno-discarded-qualifiers \
					-Wno-unused-variable -Wno-unused-parameter -Wsign-conversion -Wpointer-arith \
					-ffunction-sections -fdata-sections \
					$(ARCH_CFLAGS) $(addprefix -D,$(CDEFINES))

# Linker flags - linker script set per target
LDFLAGS := -L$(LDSCRIPT_DIR) -Wl,--gc-sections -Wl,--undefined=uxTopUsedPriority \
           --specs=nosys.specs --specs=nano.specs

# temporary build mechanism for applications: set DEFAULT_LINKER_SCRIPT=stm32f0_application.ld
DEFAULT_LINKER_SCRIPT ?= stm32f0_default.ld
ifeq ($(MAKECMDGOALS),temp-bootloader-write)
DEFAULT_LINKER_SCRIPT := stm32f0_application.ld
endif

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

# Default CAN channel for Babydriver
CHANNEL ?= can0

# Platform targets
.PHONY: program gdb target babydriver temp-bootloader-write

babydriver:
	@make program PROJECT=baby_driver
	@python3 -i projects/baby_driver/scripts/repl_setup.py --channel $(CHANNEL)

ifeq (,$(MACOS_SSH_USERNAME))

program: $(TARGET_BINARY:$(PLATFORM_EXT)=.bin)
	@$(OPENOCD) $(OPENOCD_CFG) -c "stm_flash $<" -c shutdown

gdb: $(TARGET_BINARY)
	@pkill $(OPENOCD) || true
	@setsid $(OPENOCD) $(OPENOCD_CFG) > /dev/null 2>&1 &
	@$(GDB) $< -x "$(SCRIPT_DIR)/gdb_flash"
	@pkill $(OPENOCD)

# ABSOLUTELY, COMPLETELY, ENTIRELY TEMPORARY: build + flash the bootloader preloaded with an application
# ONCE IT IS NO LONGER NECESSARY, REMOVE THIS AND ALL ITS REFERENCES
# application code starts at 16K (bootloader) + 2K (config page 1) + 2K (config page 2) = 20480
temp-bootloader-write: $(TARGET_BINARY:$(PLATFORM_EXT)=.bin) $(BIN_DIR)/bootloader.bin
	@cp $(BIN_DIR)/bootloader.bin $(BIN_DIR)/bootloader-ready.bin
	@dd if=$(TARGET_BINARY:$(PLATFORM_EXT)=.bin) of=$(BIN_DIR)/bootloader-ready.bin bs=1 seek=20480
	@$(OPENOCD) $(OPENOCD_CFG) -c "stm_flash $(BIN_DIR)/bootloader-ready.bin" -c shutdown

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
SSH_CMD := ssh -t $(MACOS_SSH_USERNAME)@$(MACOS_SSH_IP) "bash -c 'cd $(MACOS_SSH_BOX_PATH)/shared/firmware_xiv && make $(MAKECMDGOALS) $(MAKE_PARAMS)'"

.PHONY: unsupported run_ssh

# Use additional dependencies - hopefully they run first, otherwise we'll build unnecessarily
# Unfortunately this only works part of the time
test_all: unsupported
test: run_ssh

# Host is macOS so we can't pass the programmer through - do it all through SSH
program gdb temp-bootloader-write:
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
