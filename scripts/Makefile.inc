PHONY += _build
_build:

header-y := 
obj-y := 
obj-b := 
obj-l := 
clean-file ?= no

prefix := INC +=


include $(srctree)/scripts/Kbuild.include
include $(cfg_mk_file)
include Makefile

ifeq ($(wildcard $(inc_file)),)
$(error ERROR: $(inc_file) file does not exist ...)
endif

ifeq ($(clean-file),yes)
$(call clean_file,$(inc_file))
prefix := INC += 
endif

subdir-h := $(sort $(patsubst %/,%,$(filter %/,$(strip $(obj-y)))))

header-y := $(strip $(header-y))

ifneq ($(header-y),)

header-y := $(sort $(call delete_last_slash,$(header-y)))

_build : $(header-y) $(subdir-h)
	$(Q) true
	
PHONY += $(header-y)
$(header-y) : var = $(prefix) -I$(shell pwd)/$@
$(header-y) : file = $(inc_file)
$(header-y) : 
	$(call write_variable_to_file,$(var),$(file))

else

_build: $(subdir-h)
	$(Q) true

endif

PHONY += $(subdir-h)
$(subdir-h):
	$(Q) $(MAKE) $(build_inc) $@ clean-file=no
	
.PHONY : $(PHONY)