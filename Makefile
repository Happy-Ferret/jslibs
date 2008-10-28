BUILD ?= opt

SUBDIRS := libs/js src/jshost src/jsstd src/jsdebug libs/nspr src/jsio src/jsobjex src/jssqlite src/jsz src/jscrypt src/jsode src/jsimage src/jsfont src/jsprotex

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS) BUILD=$(BUILD)

.PHONY: $(MAKECMDGOALS)
$(MAKECMDGOALS):: $(SUBDIRS) ;

copy::
	-mkdir ./$(BUILD)/
	for d in $(SUBDIRS) ; do \
		cp ./$$d/$(BUILD)/* ./$(BUILD)/ ;\
	done

clean::
	-rm ./$(BUILD)/*
	-rmdir ./$(BUILD)/
