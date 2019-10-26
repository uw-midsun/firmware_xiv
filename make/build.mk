# Default build makefile
# Define TARGET as the library or project name and TARGET_TYPE as either LIB or PROJ.
# Builds the appropriate object files and creates a static library, and if applicable, an application

# Alias target so we don't have super long variable names
T := $(TARGET)
ifeq (,$(filter LIB PROJ,$(TARGET_TYPE)))
  $(error Invalid build target type for $(TARGET))
endif

# Defines default variables and targets
$(T)_DIR := $($(TARGET_TYPE)_DIR)/$(T)

$(T)_PREBUILD_CMD :=
$(T)_SRC_ROOT := $($(T)_DIR)/src
$(T)_OBJ_ROOT := $(OBJ_CACHE)/$(T)
$(T)_INC_DIRS := $($(T)_DIR)/inc $($(T)_DIR)/inc/$(PLATFORM)

$(T)_CFLAGS := $(CFLAGS)
$(T)_DEPS :=

# Include library variables - we expect to have $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS
include $($(T)_DIR)/rules.mk

# If the source or include files were not explicitly defined, use the possibly modified
# source root and include directories to find the source and headers.
ifeq (,$($(T)_SRC))
$(T)_SRC := $(wildcard $($(T)_SRC_ROOT)/*.c) \
            $(wildcard $($(T)_SRC_ROOT)/$(PLATFORM)/*.c) \
            $(wildcard $($(T)_SRC_ROOT)/*.s)
endif

# Define objects and include generated dependencies
# Note that without some very complex rules, we can only support one root source directory.
$(T)_OBJ := $($(T)_SRC:$($(T)_SRC_ROOT)/%.c=$($(T)_OBJ_ROOT)/%.o)
$(T)_OBJ := $($(T)_OBJ:$($(T)_SRC_ROOT)/%.s=$($(T)_OBJ_ROOT)/%.o)
-include $($(T)_OBJ:.o=.d) #:

# Static library link target
$(STATIC_LIB_DIR)/lib$(T).a: $($(T)_OBJ) $(call dep_to_lib,$($(T)_DEPS)) | $(STATIC_LIB_DIR)
	@echo "Linking $@"
	@$(AR) -r $@ $^

# Application target
$(BIN_DIR)/$(T)$(PLATFORM_EXT): $($(T)_OBJ) $(call dep_to_lib,$($(T)_DEPS)) | $(T) $(BIN_DIR)
	@echo "Building $(notdir $@) for $(PLATFORM)"
	@$(CC) $($(firstword $|)_CFLAGS) -Wl,-Map=$(BIN_DIR)/$(notdir $(@:%$(PLATFORM_EXT)=%.map)) $^ -o $@ \
		-L$(STATIC_LIB_DIR) $(addprefix -l,$($(firstword $|)_DEPS)) \
		$(LDFLAGS) $(addprefix -I,$($(firstword $|)_INC_DIRS))
	@$(OBJDUMP) -St $@ >$(basename $@).lst
	@$(SIZE) $@

# Object target - use first order-only dependency to expand the library name for subshells
$($(T)_OBJ_ROOT)/%.o: $($(T)_SRC_ROOT)/%.c | $(T) $(dir $($(T)_OBJ))
	@echo "$(firstword $|): $(notdir $<) -> $(notdir $@)"
	@$(CC) -MD -MP -c -o $@ $< $($(firstword $|)_CFLAGS) $(addprefix -I,$($(firstword $|)_INC_DIRS))

$($(T)_OBJ_ROOT)/%.o: $($(T)_SRC_ROOT)/%.s | $(T) $(dir $($(T)_OBJ))
	@echo "$(firstword $|): $(notdir $<) -> $(notdir $@)"
	@$(CC) -MD -MP -c -o $@ $< $($(firstword $|)_CFLAGS) $(addprefix -I,$($(firstword $|)_INC_DIRS))

.PHONY: $(T) $(TARGET_TYPE)

# Postpone dependency and include directory resolution
# We use make's dependency resolution to ensure that the libraries and projects
# are resolved in the correct order.
$(T): $($(T)_DEPS) | $(TARGET_TYPE)
	$(eval $(@)_DEPS += $(foreach dep,$^,$($(dep)_DEPS)))
	$(eval $(@)_INC_DIRS += $(LIB_INC_DIRS))
	@echo "Processing $(firstword $|) $@"
	@$($(@)_PREBUILD_CMD)

$(TARGET_TYPE):

ifneq (unity,$(T))
  include $(MAKE_DIR)/build_test.mk
endif

ifeq (LIB,$(TARGET_TYPE))
  LIB_INC_DIRS := $(sort $(LIB_INC_DIRS) $($(T)_INC_DIRS))
endif

DIRS := $(sort $(DIRS) $($(T)_OBJ_DIR) $(dir $($(T)_OBJ)))
INC_DIRS := $(sort $(INC_DIRS) $($(T)_INC_DIRS))
