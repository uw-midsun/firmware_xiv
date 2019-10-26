# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

$(T)_DEPS :=

# GIT_VERSION_COMMIT_HASH is an 8-digit git commit hash
GIT_VERSION_COMMIT_HASH := $(shell git rev-parse --short HEAD)
# GIT_VERSION_DIRTY_STATUS denotes whether or not the tree is dirty
GIT_VERSION_DIRTY_STATUS := "clean"

# We define a tree as "dirty" if there exist any possible changes that might
# affect our build outputs. Formally, the tree is considered "dirty" if there
# are any:
#
# 1. Staged changes
# 2. Unstaged changes
# 3. Untracked changes in files that are not ignored
#
# This is so we don't need to parse the dependencies of each project and their
# transitive dependencies.
#
# Theoretically, we should just be able to get away with running the porcelain
# version of git status. Otherwise, we can do the same thing, but with plumbing
# commands.
#
# Staged changes
# git diff-index --quiet --cached HEAD --
# Return:
#  - 0 if no changes
#  - 1 if changes
#
# Unstaged changes:
# git diff-files --quiet
# Return:
#  - 0 if no unstaged changes
#  - 1 if unstaged changes
#
# Untracked changes
#  git ls-files --exclude-standard --others .
#
# For the sake of simplicity, we're just going to use the porcelain commands,
# and if we run into issues down the road, we know what to fix.
ifneq ($(strip $(shell git status --porcelain 2>/dev/null)),)
  GIT_VERSION_DIRTY_STATUS := "dirty"
endif

# Basically what we're doing here is generating a temporary file and then
# comparing it against the current header. This is because Make can't track
# changes to CFLAGS, and thus, we would have to touch the header each time.
# This is an attempt at reducing the amount of time we spend rebuilding due to
# changes in this header.
#
# If the header file is different, then we're going to replace it with the
# newly generated header, which will trigger a rebuild.
#
# Otherwise, the header file is the same, and so we clean up the temporary
# file, and no rebuild is necessary.
$($(T)_DIR)/inc/git_version_impl.h: .FORCE
	@echo "Version: $(GIT_VERSION_COMMIT_HASH)-$(GIT_VERSION_DIRTY_STATUS)"
	@echo "#pragma once" >| $(dir $@)/git_version_impl.h.tmp; \
	echo "#define GIT_VERSION_COMMIT_HASH \"${GIT_VERSION_COMMIT_HASH}\"" >> $(dir $@)/git_version_impl.h.tmp; \
	echo "#define GIT_VERSION_DIRTY_STATUS \"${GIT_VERSION_DIRTY_STATUS}\"" >> $(dir $@)/git_version_impl.h.tmp;
	@if ! [ -f $(dir $@)/git_version_impl.h ] || ! cmp -s $(dir $@)/git_version_impl.h.tmp $(dir $@)/git_version_impl.h; then \
		echo "Replacing git_version_impl.h"; \
		mv $(dir $@)/git_version_impl.h.tmp $(dir $@)/git_version_impl.h; \
	else \
		rm -f $(dir $@)/git_version_impl.h.tmp; \
	fi

$($(T)_OBJ_ROOT)/git_version.o: $($(T)_DIR)/inc/git_version_impl.h $($(T)_DIR)/inc/git_version.h
