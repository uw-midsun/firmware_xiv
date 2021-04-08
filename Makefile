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
#   DF: [DEFINE=] - Specifies space-separated preprocessor symbols to define.
#   CH: [CHANNEL=] - Specifies the default CAN channel for Babydriver. Defaults to vcan0 on x86 and can0 on stm32f0xx.
#
# Usage:
#   make [all] [PL] [PR] [DF] - Builds the target project and its dependencies
#   make clean - Completely deletes all build output
#   make format - Formats all non-vendor code
#   make gdb [PL] [PR|LI] [TE] [DF] - Builds and runs the specified unit test and connects an instance of GDB
#   make lint - Lints all non-vendor code
#   make new [PR|LI] - Creates folder structure for new project or library
#   make remake [PL] [PR] [DF] - Cleans and rebuilds the target project (does not force-rebuild dependencies)
#   make test [PL] [PR|LI] [TE] [DF] - Builds and runs the specified unit test, assuming all tests if TE is not defined
#   make pytest [PR] [TE] - Runs the specified python unit test, assuming all tests in scripts directory of a project if TE is not defined
#   make pytest_all - Runs all python tests in the scripts directory of every project 
#   make install_requirements - Installs python requirements for every project
#   make codegen - Generates header files for CAN messages used in firmware
# 	make codegen_dbc - Generates a DBC file from protobuf / .asciipb file
# 	make codegen_protos - Generates protobuf files 
# 	make mock_can_data - Mocks CAN data based off DBC file to the CAN bus on x86 
#   make babydriver [PL] [CH] - Flash or run the Babydriver debug project and drop into its Python shell
#   make mpxe [TE] - Build and run the specified MPXE integration test, or all integration tests if TE is not defined
#   make fastmpxe [TE] - Don't build and just run the MPXE integration test, or all if TE is not defined.
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
CODEGEN_DIR := codegen
MPXE_DIR := mpxe

ifeq ($(MAKECMDGOALS),mpxe)
PLATFORM ?= x86
DEFINE += MPXE
IS_MPXE := TRUE
$(call gen_mpxe)
else
PLATFORM ?= stm32f0xx
endif

# Include argument filters
include $(MAKE_DIR)/filter.mk

# Location of project
PROJECT_DIR := $(PROJ_DIR)/$(PROJECT)

# Location of library
LIBRARY_DIR := $(LIB_DIR)/$(LIBRARY)

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

# Dependable variable directory
DEP_VAR_DIR := $(BUILD_DIR)/dep_var/$(PLATFORM)

# Set target binary - invalid for targets with more than one binary
ifeq (,$(TEST))
TARGET_BINARY = $(BIN_DIR)/$(PROJECT)$(PLATFORM_EXT)
else
TARGET_BINARY = $(BIN_DIR)/test/$(LIBRARY)$(PROJECT)/test_$(TEST)_runner$(PLATFORM_EXT)
endif

# MPXE generated file directories
MPXE_C_GEN_DIR := $(LIB_DIR)/mpxe-gen
MPXE_PYTHON_GEN_DIR := $(MPXE_DIR)/protogen
MPXE_PROTOS_DIR := $(MPXE_DIR)/protos

DIRS := $(BUILD_DIR) $(BIN_DIR) $(STATIC_LIB_DIR) $(OBJ_CACHE) $(DEP_VAR_DIR)
COMMA := ,

# Virtualenv directory
VENV_DIV := .venv

# Please don't touch anything below this line
###################################################################################################

# AUTOMATED ACTIONS

# $(call gen_mpxe)
define gen_mpxe
$(shell mkdir -p $(MPXE_C_GEN_DIR)/inc $(MPXE_C_GEN_DIR)/src $(MPXE_PYTHON_GEN_DIR))
$(shell cd $(MPXE_PROTOS_DIR) && protoc --c_out=$(ROOT)/$(MPXE_C_GEN_DIR)/inc *)
$(shell cd $(MPXE_PROTOS_DIR) && protoc --python_out=$(ROOT)/$(MPXE_PYTHON_GEN_DIR) *)
$(shell mv $(MPXE_C_GEN_DIR)/inc/*.c $(MPXE_C_GEN_DIR)/src)
$(shell rm -r -f $(MPXE_C_GEN_DIR)/inc/mpxe)
endef

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

# Hack to enable depending on a variable - https://stackoverflow.com/a/26147844
# $(eval $(call dependable_var,variable))
DEP_VARS :=
define dependable_var
DEP_VARS += $(DEP_VAR_DIR)/$1
.PHONY: phony
$(DEP_VAR_DIR)/$1: phony | $(DEP_VAR_DIR)
	@if [ "`cat $(DEP_VAR_DIR)/$1 2>&1`" != '$($1)' ]; then \
		echo -n $($1) > $(DEP_VAR_DIR)/$1; \
	fi
endef

# include the target build rules
-include $(PROJECT_DIR)/rules.mk

###################################################################################################

# ENV SETUP

ROOT := $(shell pwd)

###################################################################################################

# PIP ENVIRONMENT SETUP


export PATH := $(ROOT)/$(VENV_DIV)/bin:$(PATH)

###################################################################################################

# MAKE PROJECT

# Actually calls the make
.PHONY: all
all: build lint pylint

# Includes platform-specific configurations
include $(PLATFORMS_DIR)/$(PLATFORM)/platform.mk

# Adds preprocessor defines
CFLAGS += $(addprefix -D,$(DEFINE))

# Allow depending on the value of DEFINE so we rebuild after changing defines
$(eval $(call dependable_var,DEFINE))

ifneq (,$(IS_MPXE))
$(eval $(call gen_mpxe))
endif

# Includes all libraries so make can find their targets
$(foreach lib,$(VALID_LIBRARIES),$(call include_lib,$(lib)))

# Includes all projects so make can find their targets
$(foreach proj,$(VALID_PROJECTS),$(call include_proj,$(proj)))

IGNORE_CLEANUP_LIBS := CMSIS FreeRTOS STM32F0xx_StdPeriph_Driver unity FatFs mpxe-gen
# This uses regex
IGNORE_PY_FILES := ./lint.py ./libraries/unity ./.venv
# Find all python files excluding ignored files
IGNORE_TO_FIND_CMD := $(foreach dir, $(IGNORE_PY_FILES), $(if $(findstring $(lastword $(IGNORE_PY_FILES)), $(dir)), -path $(dir), -path $(dir) -o))
FIND_PY_FILES:= $(shell printf "! -regex %s " $(IGNORE_PY_FILES) | xargs find . \( $(IGNORE_TO_FIND_CMD) \) -prune -o -name '*.py' -print)
AUTOPEP8_CONFIG:= -a --max-line-length 100 -r
FIND_PATHS := $(addprefix -o -path $(LIB_DIR)/,$(IGNORE_CLEANUP_LIBS))
FIND := find $(PROJECT_DIR) $(LIBRARY_DIR) \
			  \( $(wordlist 2,$(words $(FIND_PATHS)),$(FIND_PATHS)) \) -prune -o \
				-iname "*.[ch]" -print
FIND_MOD_NEW := git diff origin/master --name-only --diff-filter=ACMRT -- '*.c' '*.h' ':(exclude)*.mako.*'
# ignore MPXE since it has a different pylint
FIND_MOD_NEW_PY := git diff origin/master --name-only --diff-filter=ACMRT -- '*.py' ':(exclude)mpxe/*.py' ':(exclude)./.venv/*'
FIND_MOD_NEW_MPXE_PY := git diff origin/master --name-only --diff-filter=ACMRT -- 'mpxe/*.py' ':(exclude)./.venv/*'

# Lints libraries and projects, excludes IGNORE_CLEANUP_LIBS
.PHONY: lint
lint:
	@echo "Linting *.[ch] in $(PROJECT_DIR), $(LIBRARY_DIR)"
	@echo "Excluding libraries: $(IGNORE_CLEANUP_LIBS)"
	@$(FIND) | xargs -r python2 lint.py

# Quick lint on ONLY changed/new files
.PHONY: lint_quick
lint_quick:
	@echo "Quick linting on ONLY changed/new C files"
	@$(FIND_MOD_NEW) | xargs -r python2 lint.py

# Globally disable the following pylint messages:
PYLINT_DISABLE := \
	import-error redefined-outer-name unused-argument \
	too-few-public-methods duplicate-code no-self-use

# Disable these additional pylint messages for MPXE:
MPXE_PYLINT_DISABLE := \
	missing-module-docstring missing-class-docstring \
	missing-function-docstring invalid-name

PYLINT := pylint $(addprefix --disable=,$(PYLINT_DISABLE))
MPXE_PYLINT := pylint $(addprefix --disable=,$(PYLINT_DISABLE)) $(addprefix --disable=,$(MPXE_PYLINT_DISABLE))

# Lints Python files, excluding MPXE generated files
.PHONY: pylint
pylint:
	@echo "Linting *.py in $(MAKE_DIR), $(PLATFORMS_DIR), $(PROJECT_DIR), $(LIBRARY_DIR), $(MPXE_DIR), $(CODEGEN_DIR)"
	@echo "Excluding libraries: $(IGNORE_CLEANUP_LIBS)"
	@find $(MAKE_DIR) $(PLATFORMS_DIR) $(CODEGEN_DIR)/scripts -iname "*.py" -print | xargs -r $(PYLINT)
	@$(FIND:"*.[ch]"="*.py") | xargs -r $(PYLINT)
	@find $(MPXE_DIR) -path $(MPXE_PYTHON_GEN_DIR) -prune -o -iname "*.py" -print | xargs -r $(MPXE_PYLINT)

.PHONY: pylint_quick
pylint_quick:
	@echo "Quick linting ONLY changed/new Python files"
	@$(FIND_MOD_NEW_PY) | xargs -r $(PYLINT)
	@$(FIND_MOD_NEW_MPXE_PY) | xargs -r $(MPXE_PYLINT)

.PHONY: format_quick
format_quick:
	@echo "Quick format on ONlY changed/new C files"
	@$(FIND_MOD_NEW) | xargs -r clang-format -i -style=file
	
.PHONY: pyformat_quick
pyformat_quick: 
	@echo "Quick format on ONLY changed/new Python files"
	@$(FIND_MOD_NEW_PY) | xargs -r autopep8 $(AUTOPEP8_CONFIG) -i
	@$(FIND_MOD_NEW_MPXE_PY) | xargs -r autopep8 $(AUTOPEP8_CONFIG) -i

# Formats libraries and projects, excludes IGNORE_CLEANUP_LIBS
.PHONY: format
format:
	@echo "Formatting *.[ch] in $(PROJECT_DIR), $(LIBRARY_DIR)"
	@echo "Excluding libraries: $(IGNORE_CLEANUP_LIBS)"
	@$(FIND) | xargs -r clang-format -i -style=file

.PHONY: pyformat
pyformat: 
	@echo "Formatting all *.py files in repo"
	@echo "Excluding: $(IGNORE_PY_FILES)"
	@autopep8 $(AUTOPEP8_CONFIG) -i $(FIND_PY_FILES)

# Note: build.py relies on a lot of relative paths so it would be easier to just cd and execute command 
.PHONY: codegen
codegen: codegen_protos
	@echo "Generating from templates..."
	@cd $(CODEGEN_DIR) && python scripts/build.py 
	@find $(CODEGEN_DIR)/out -type f \( -iname '*.[ch]' \) | xargs -r clang-format -i -fallback-style=Google
	@find $(CODEGEN_DIR)/out -name \*.h -exec cp {} libraries/codegen-tooling/inc/ \;

# Note: build_dbc has same issue as build.py with local paths
.PHONY: codegen_dbc
codegen_dbc:
	@echo "Generating DBC file"
	@cd $(CODEGEN_DIR) && python scripts/build_dbc.py

.PHONY: codegen_protos 
codegen_protos:
	@echo "Compiling protos..."
	@mkdir -p $(CODEGEN_DIR)/genfiles
	@protoc -I=$(CODEGEN_DIR)/schema --python_out=$(CODEGEN_DIR)/genfiles $(CODEGEN_DIR)/schema/can.proto

.PHONY: pytest
pytest:
	@python3 -m unittest discover -t $(PROJ_DIR)/$(PROJECT)/scripts -s $(PROJ_DIR)/$(PROJECT)/scripts -p "test_*$(TEST).py"

.PHONY: pytest_all
pytest_all:
	@for i in $$(find . -path ./$(VENV_DIV) -prune -o -path ./mpxe/integration_tests -prune -o -name "test_*.py"); 			\
	do																								\
		python -m unittest discover -t $$(dirname $$i) -s $$(dirname $$i) -p $$(basename $$i);		\
	done	

# Tests that all files have been run through the format target mainly for CI usage
.PHONY: test_format
test_format: format
	@! git diff --name-only --diff-filter=ACMRT | xargs -r -n1 clang-format -style=file -output-replacements-xml | grep '<replacements' > /dev/null; if [ $$? -ne 0 ] ; then git --no-pager diff && exit 1 ; fi
	@! git diff --name-only --diff-filter=ACMRT -- '*.py' | xargs -r -n1 autopep8 $(AUTOPEP8_CONFIG) -d | grep '@@' > /dev/null; if [ $$? -ne 0 ] ; then git --no-pager diff && exit 1 ; fi

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
	@echo cleaning
	@rm -rf $(BUILD_DIR)
	@rm -f $(LIB_DIR)/mpxe-gen/inc/*.pb-c.h
	@rm -f $(LIB_DIR)/mpxe-gen/src/*.pb-c.c
	@rm -f $(MPXE_DIR)/protogen/*_pb2.py

.PHONY: mock_can_data
mock_can_data: socketcan
	@cd $(CODEGEN_DIR) && python3 mock_can_data.py

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

# Note: ". .venv/bin/activate" is the /sh/ (and more portable way) of bash's "source .venv/bin/activate"
.PHONY: install_requirements
install_requirements:
	@sudo add-apt-repository ppa:maarten-fonville/protobuf -y
	@sudo apt-get update
	@sudo apt-get install protobuf-compiler
	@rm -rf $(VENV_DIV)
	@mkdir $(VENV_DIV)
	@virtualenv $(VENV_DIV)
	@. $(VENV_DIV)/bin/activate; \
	pip install -r requirements.txt

MPXE_PROJS := 
-include $(MPXE_DIR)/integration_tests/deps.mk

.PHONY: fastmpxe
fastmpxe:
	@python3 -m unittest discover -t $(MPXE_DIR) -s $(MPXE_DIR)/integration_tests -p "test_*$(TEST).py"

.PHONY: mpxe
mpxe: $(MPXE_PROJS:%=$(BIN_DIR)/%) socketcan fastmpxe

# Dummy force target for pre-build steps
.PHONY: .FORCE
