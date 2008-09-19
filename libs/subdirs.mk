ifeq ($(MAKECMDGOALS),)
$(error NO GOAL SPECIFIED)
endif

BUILD ?= opt
BITS ?= 32

.PHONY: $(MAKECMDGOALS)

.DEFAULT:
	echo IGNORE $@

DEST_DIR = ./$(BUILD)/

.PHONY: copy
copy:
	mkdir -p $(DEST_DIR)
	cp $(TARGET_FILES) $(DEST_DIR)

clean::
	-rm $(DEST_DIR)*
	-rmdir $(DEST_DIR)
