ifeq ($(MAKECMDGOALS),)
$(error NO GOAL SPECIFIED)
endif

BUILD?=opt
BITS?=32

ifeq ($(shell uname -o),Msys)
	INT_DIR=Win32_$(BUILD)/
else
	INT_DIR=$(shell uname)_$(BUILD)_$(BITS)/
endif

.PHONY: $(MAKECMDGOALS)

.DEFAULT:
	echo IGNORE $@

$(DEST_DIR):
	mkdir -p $(DEST_DIR)

copy: $(DEST_DIR)
	-[ -n "$(DEST_DIR)" ] && cp $(TARGET_FILES) $(DEST_DIR)

clean::
	echo clearing $(DEST_DIR)
	#-[ -n "$(DEST_DIR)" ] && rm $(DEST_DIR)* && rmdir $(DEST_DIR)
