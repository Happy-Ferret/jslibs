BUILD ?= opt

SUBDIRS = js jshost jsstd nspr jsio jsobjex jsz jscrypt

all clean distclean copy:
	for d in $(SUBDIRS); do $(MAKE) -C $$d $@ BUILD=$(BUILD); done

install: copy
	-mkdir ./$(BUILD)/
	for d in $(SUBDIRS); do cp $$d/$(BUILD)/* ./$(BUILD)/; done


################################ END
