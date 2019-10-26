###################################################################################################
# Midnight Sun's build system
#
# Arguments:
#   PL: [PLATFORM=] - Specifies the target platform (based on device family, defaults to stm32f0xx)
#   PR: [PROJECT=] - Specifies the target project
#   LI: [LIBRARY=] - Specifies the target library (only valid for tests)
#   TE: [TEST=] - Specifies the target test (only valid for tests, requires LI or PR to be specified)
#   CM: [COMPILER=] - Specifies the compiler to use on x86. Defaults to gcc [gcc | clang].
#   CO: [COPTIONS=] - Specifies compiler options on x86 [asan | tsan].
#   PB: [PROBE=] - Specifies which debug probe to use on STM32F0xx. Defaults to cmsis-dap [cmsis-dap | stlink-v2].
#
# Usage:
#   make [all] [PL] [PR] - Builds the target project and its dependencies
#   make clean - Completely deletes all build output
#   make format - Formats all non-vendor code
#   make gdb [PL] [PR|LI] [TE] - Builds and runs the specified unit test and connects an instance of GDB
#   make lint - Lints all non-vendor code
#   make new [PR|LI] - Creates folder structure for new project or library
#   make remake [PL] [PR] - Cleans and rebuilds the target project (does not force-rebuild dependencies)
#   make test [PL] [PR|LI] [TE] - Builds and runs the specified unit test, assuming all tests if TE is not defined
#   make update_codegen - Update the codegen-tooling release
#
# Platform specific:
#   make gdb [PL=stm32f0xx] [PL] [PR] [PB]
#   make program [PL=stm32f0xx] [PR] [PB] - Programs and runs the project through OpenOCD
#   make <build | test | remake | all> [PL=x86] [CM=clang [CO]]
#
###################################################################################################

# CONFIG

# Default directories
PROJ_DIR := projects
PLATFORMS_DIR := platform
LIB_DIR := libraries
MAKE_DIR := make

PLATFORM ?= stm32f0xx

# Include argument filters
include $(MAKE_DIR)/filter.mk

# Location of project
PROJECT_DIR := $(PROJ_DIR)/$(PROJECT)

# Location of platform
PLATFORM_DIR := $(PLATFORMS_DIR)/$(PLATFORM)

# Output directory
BUILD_DIR := build

# compile directory
BIN_DIR := $(BUILD_DIR)/bin/$(PLATFORM)

# Static library directory
STATIC_LIB_DIR := $(BUILD_DIR)/lib/$(PLATFORM)

# Object cache
OBJ_CACHE := $(BUILD_DIR)/obj/$(PLATFORM)

# Set target binary - invalid for targets with more than one binary
ifeq (,$(TEST))
TARGET_BINARY = $(BIN_DIR)/$(PROJECT)$(PLATFORM_EXT)
else
TARGET_BINARY = $(BIN_DIR)/test/$(LIBRARY)$(PROJECT)/test_$(TEST)_runner$(PLATFORM_EXT)
endif

DIRS := $(BUILD_DIR) $(BIN_DIR) $(STATIC_LIB_DIR) $(OBJ_CACHE)
COMMA := ,

# Please don't touch anything below this line
###################################################################################################

# AUTOMATED ACTIONS

# $(call include_lib,libname)
define include_lib
$(eval TARGET := $(1));
$(eval TARGET_TYPE := LIB);
$(eval include $(MAKE_DIR)/build.mk);
$(eval undefine TARGET; undefine TARGET_TYPE)
endef

# $(call include_proj,projname)
define include_proj
$(eval TARGET := $(1));
$(eval TARGET_TYPE := PROJ);
$(eval include $(MAKE_DIR)/build.mk);
$(eval undefine TARGET; undefine TARGET_TYPE)
endef

# $(call dep_to_lib,deps)
define dep_to_lib
$(1:%=$(STATIC_LIB_DIR)/lib%.a)
endef
.PHONY: # Just adding a colon to fix syntax highlighting

# $(call find_in,folders,wildcard)
define find_in
$(foreach folder,$(1),$(wildcard $(folder)/$(2)))
endef

# include the target build rules
-include $(PROJECT_DIR)/rules.mk

###################################################################################################

# ENV SETUP

ROOT := $(shell pwd)

###################################################################################################

# MAKE PROJECT

# Actually calls the make
.PHONY: all
all: build lint pylint

# Includes platform-specific configurations
include $(PLATFORMS_DIR)/$(PLATFORM)/platform.mk

# Includes all libraries so make can find their targets
$(foreach lib,$(VALID_LIBRARIES),$(call include_lib,$(lib)))

# Includes all projects so make can find their targets
$(foreach proj,$(VALID_PROJECTS),$(call include_proj,$(proj)))

IGNORE_CLEANUP_LIBS := CMSIS FreeRTOS STM32F0xx_StdPeriph_Driver unity FatFs
FIND_PATHS := $(addprefix -o -path $(LIB_DIR)/,$(IGNORE_CLEANUP_LIBS))
FIND := find $(PROJ_DIR) $(LIB_DIR) \
			  \( $(wordlist 2,$(words $(FIND_PATHS)),$(FIND_PATHS)) \) -prune -o \
				-iname "*.[ch]" -print

# Lints libraries and projects, excludes IGNORE_CLEANUP_LIBS
.PHONY: lint
lint:
	@echo "Linting *.[ch] in $(PROJ_DIR), $(LIB_DIR)"
	@echo "Excluding libraries: $(IGNORE_CLEANUP_LIBS)"
	@$(FIND) | xargs -r python2 lint.py

# Disable import error
.PHONY: pylint
pylint:
	@echo "Linting *.py in $(MAKE_DIR), $(PLATFORMS_DIR), $(PROJ_DIR), $(LIB_DIR)"
	@echo "Excluding libraries: $(IGNORE_CLEANUP_LIBS)"
	@find $(MAKE_DIR) $(PLATFORMS_DIR) -iname "*.py" -print | xargs -r pylint --disable=F0401 --disable=duplicate-code
	@$(FIND:"*.[ch]"="*.py") | xargs -r pylint --disable=F0401 --disable=duplicate-code

# Formats libraries and projects, excludes IGNORE_CLEANUP_LIBS
.PHONY: format
format:
	@echo "Formatting *.[ch] in $(PROJ_DIR), $(LIB_DIR)"
	@echo "Excluding libraries: $(IGNORE_CLEANUP_LIBS)"
	@$(FIND) | xargs -r clang-format -i -style=file

# Tests that all files have been run through the format target mainly for CI usage
.PHONY: test_format
test_format: format
	@! git diff --name-only --diff-filter=ACMRT | xargs -n1 clang-format -style=file -output-replacements-xml | grep '<replacements' > /dev/null; if [ $$? -ne 0 ] ; then git --no-pager diff && exit 1 ; fi

# Builds the project or library
.PHONY: build
ifneq (,$(PROJECT)$(TEST))
build: $(TARGET_BINARY)
else
build: $(STATIC_LIB_DIR)/lib$(LIBRARY).a
endif

# Assumes that all libraries are used and will be built along with the projects
.PHONY: build_all
build_all: $(VALID_PROJECTS:%=$(BIN_DIR)/%$(PLATFORM_EXT))

$(DIRS):
	@mkdir -p $@

$(BIN_DIR)/%.bin: $(BIN_DIR)/%$(PLATFORM_EXT)
	@$(OBJCPY) -O binary $< $(<:$(PLATFORM_EXT)=.bin)

###################################################################################################

# EXTRA

# Creates folder structure for a new project or library
.PHONY: new
new:
	@python3 $(MAKE_DIR)/new_target.py $(NEW_TYPE) $(PROJECT)$(LIBRARY)

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)

.PHONY: remake
remake: clean all

.PHONY: socketcan
socketcan:
	@sudo modprobe can
	@sudo modprobe can_raw
	@sudo modprobe vcan
	@sudo ip link add dev vcan0 type vcan || true
	@sudo ip link set up vcan0 || true
	@ip link show vcan0

.PHONY: update_codegen
update_codegen:
	@python make/git_fetch.py -folder=libraries/codegen-tooling -user=uw-midsun -repo=codegen-tooling -tag=latest -file=codegen-tooling-out.zip

# Dummy force target for pre-build steps
.PHONY: .FORCE
