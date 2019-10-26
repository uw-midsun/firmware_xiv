VALID_PROJECTS := $(patsubst $(PROJ_DIR)/%/rules.mk,%,$(wildcard $(PROJ_DIR)/*/rules.mk))
VALID_PLATFORMS := $(patsubst $(PLATFORMS_DIR)/%/platform.mk,%,$(wildcard $(PLATFORMS_DIR)/*/platform.mk))
VALID_LIBRARIES := $(patsubst $(LIB_DIR)/%/rules.mk,%,$(wildcard $(LIB_DIR)/*/rules.mk))

# Rules:
# - For any universal operations (clean, lint), do not expect args
# - For build/test all, just check for a valid PLATFORM.
# - If new project, just make sure one of PROJECT or LIBRARY is defined
# - For test, gdb, and program, check to see if PLATFORM and {PROJECT or {LIBRARY and TEST}} are valid
# - For build, check if PLATFORM and {PROJECT or LIBRARY} are valid

ifneq (,$(filter clean lint pylint format build_all test_all test_format socketcan update_codegen,$(MAKECMDGOALS)))
  # Universal operation: do nothing - args are not used or only PLATFORM is checked
else ifneq (,$(filter new,$(MAKECMDGOALS)))
  # New project: just make sure PROJECT or LIBRARY is defined
  ifeq (,$(PROJECT)$(LIBRARY))
    $(error Missing project or library name. Expected PROJECT=... or LIBRARY=...)
  endif

  NEW_TYPE := project
  ifeq (,$(PROJECT))
    NEW_TYPE := library
  endif
else
  # Test/GDB/program/build: check for valid PROJECT or LIBRARY
  override PROJECT := $(filter $(VALID_PROJECTS),$(PROJECT))
  override LIBRARY := $(filter $(VALID_LIBRARIES),$(LIBRARY))

  ifeq (,$(PROJECT)$(LIBRARY))
    $(error Invalid project or library. Expected PROJECT=[$(VALID_PROJECTS)] or LIBRARY=[$(VALID_LIBRARIES)])
  endif
endif

ifneq (,$(filter build_all test_all test gdb program build,$(MAKECMDGOALS)))
  # Check for valid PLATFORM
  override PLATFORM := $(filter $(VALID_PLATFORMS),$(PLATFORM))

  ifeq (,$(PLATFORM))
    $(error Invalid platform. Expected PLATFORM=[$(VALID_PLATFORMS)])
  endif
endif
