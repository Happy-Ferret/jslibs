ifeq ($(BUILD),dbg)
        LIBJS_DIR = Linux_All_DBG.OBJ
        LIBJS_MAKE_OPTIONS =
        JSLIB_MAKE_OPTIONS = BUILD=$(BUILD)
        BUILD_DIR = dbg
else
        LIBJS_DIR = Linux_All_OPT.OBJ
        LIBJS_MAKE_OPTIONS = BUILD_OPT=1
        JSLIB_MAKE_OPTIONS =
        BUILD_DIR = opt
endif


js/src/$(LIBJS_DIR)/libjs.so:
        cd js/src && $(MAKE) -f Makefile.ref $(LIBJS_MAKE_OPTIONS)/

%::
        cd $(dir $@) && $(MAKE) $(JSLIB_MAKE_OPTIONS)

.PHONY: all
all: js/src/$(LIBJS_DIR)/libjs.so moduleManager/moduleManager.a jshost/jshost jsstd/jsstd.so jsio/jsio.so jsobjex/jsobjex.so
