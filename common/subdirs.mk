ifeq ($(MAKECMDGOALS),)
$(error NO GOAL SPECIFIED)
endif

.PHONY: $(MAKECMDGOALS)

.DEFAULT:
	echo IGNORE $@

BUILD ?= opt

DEST_DIR = ./$(BUILD)/

.PHONY: copy
copy:
	-mkdir $(DEST_DIR)
	cp $(TARGET_FILES) $(DEST_DIR)
