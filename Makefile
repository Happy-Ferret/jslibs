BUILD ?= opt

SUBDIRS := js jshost jsstd nspr jsio jsobjex jssqlite jsz jscrypt

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
