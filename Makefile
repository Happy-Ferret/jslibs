BUILD ?= opt

SUBDIRS = js jshost jsstd nspr jsio jsobjex jssqlite jsz jscrypt

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ BUILD=$(BUILD) $(MAKECMDGOALS)

$(MAKECMDGOALS): $(SUBDIRS) ;

install:
	-mkdir ./$(BUILD)/
	for d in $(SUBDIRS); do \
		$(MAKE) -C $$d BUILD=$(BUILD) copy && cp $$d/$(BUILD)/* ./$(BUILD)/ ;\
	done

