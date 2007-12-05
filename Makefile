ifeq ($(BUILD),dbg)
	BUILD = dbg
	LIBJS_DIR = Linux_All_DBG.OBJ
	LIBJS_MAKE_OPTIONS =
else
	BUILD = opt
	LIBJS_DIR = Linux_All_OPT.OBJ
	LIBJS_MAKE_OPTIONS = BUILD_OPT=1
endif

FILES = \
	js/src/$(LIBJS_DIR)/libjs.so\
	jshost/jshost\
	jsstd/jsstd.so\
	jsio/jsio.so\
	jsobjex/jsobjex.so

js/src/$(LIBJS_DIR)/libjs.so:
	cd js/src && $(MAKE) -f Makefile.ref $(LIBJS_MAKE_OPTIONS)

$(FILES):
	$(MAKE) -C $(dir $@) $(MAKECMDGOALS)

.PHONY: all clean install

all: $(FILES)

clean:
	rm $(FILES)

install: all
	-mkdir ./$(BUILD)
	cp $(FILES) ./$(BUILD)
	cp ./nspr/nsprpub/dist/lib/libnspr4.so ./$(BUILD)



######################### END

#  for d in $(DEPENDS); do $(MAKE) -C $$d $(MAKECMDGOALS); done

