BUILD ?= opt

SUBDIRS = js jshost jsstd nspr jsio jsobjex jssqlite jsz jscrypt

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ BUILD=$(BUILD) $(MAKECMDGOALS)

$(MAKECMDGOALS) copy: $(SUBDIRS) ;

install: copy
	-mkdir ./$(BUILD)/
	for d in $(SUBDIRS); do cp $$d/$(BUILD)/* ./$(BUILD)/; done

