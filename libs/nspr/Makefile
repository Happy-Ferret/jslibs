include ../subdirs.mk

ifeq ($(BUILD),dbg)
	_CONFIGURE_OPTIONS = --enable-debug --disable-optimize --enable-debug-rtl
else
	_CONFIGURE_OPTIONS = --disable-debug --enable-optimize --enable-strip
endif

ifeq ($(BITS),64)
	_CONFIGURE_OPTIONS += --enable-64bit
endif

TARGET_FILES=$(INT_DIR)dist/lib/*nspr4.{so,dll}

$(INT_DIR)Makefile:
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && ../src/configure $(_CONFIGURE_OPTIONS) --enable-win32-target=WIN95

all clean:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@

clean::
	-[ -d "$(INT_DIR)" ] && rm -r ./$(INT_DIR)
