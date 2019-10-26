$(T)_DEPS := ms-common ms-helper

ifeq (x86,$(PLATFORM))
$(T)_EXCLUDE_TESTS := sd
endif
