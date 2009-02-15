BUILD ?= opt
BITS ?= 32

SUBDIRS := libs/js libs/nspr \
           src/jshost \
           src/jsstd src/jsdebug src/jsio src/jsobjex src/jssqlite src/jsz src/jscrypt src/jstask src/jsimage src/jsfont src/jsprotex src/jsfastcgi src/jsiconv 

DEST_DIR=$(PWD)/$(shell uname)_$(BITS)_$(BUILD)/

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS) BUILD=$(BUILD) BITS=$(BITS) DEST_DIR=$(DEST_DIR)

.PHONY: $(MAKECMDGOALS)
$(MAKECMDGOALS):: $(SUBDIRS) ;

clean::
	-rm ./$(BUILD)/*
	-rmdir ./$(BUILD)/
