ifeq ($(MAKECMDGOALS),)
$(error NO GOAL SPECIFIED)
endif

BUILD?=opt
BITS?=32

# INT_DIR (intermediate directory) may not be used by the Makefile, it's just a helper. It's up to the Makefile to set the TARGET_FILES variable
INT_DIR=$(shell uname)_$(BUILD)_$(BITS)/

.PHONY: $(MAKECMDGOALS)

.DEFAULT:
	echo IGNORE $@

$(DEST_DIR):
	mkdir -p $(DEST_DIR)

copy: $(DEST_DIR)
	cp $(TARGET_FILES) $(DEST_DIR)

clean::
	echo clearing $(DEST_DIR)
	#-[ -n "$(DEST_DIR)" ] && rm $(DEST_DIR)* && rmdir $(DEST_DIR)
