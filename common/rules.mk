ifeq ($(MAKECMDGOALS),)
$(error NO GOAL SPECIFIED)
endif

BUILD ?= opt
BITS ?= 32

ifeq ($(BUILD),dbg)
	CFLAGS += -Wall -g3 -O0 -DDEBUG
	SMINC = -I../../libs/js/src/Linux_All_DBG.OBJ -I../../libs/js/src
	SMLIB = -Wl,-Bdynamic -L../../libs/js/src/Linux_All_DBG.OBJ -ljs
	SMDEF = -DJS_GCMETER -DJS_HASHMETER -DJS_GC_ZEAL -DJS_DUMP_PROPTREE_STATS
#  -DJS_ARENAMETER
else
	CFLAGS += -Wall -O3 -s -funroll-loops
	SMINC = -I../../libs/js/src/Linux_All_OPT.OBJ -I../../libs/js/src
	SMLIB = -Wl,-Bdynamic -L../../libs/js/src/Linux_All_OPT.OBJ -ljs
	SMDEF =
endif


ifneq ($(findstring .so,$(TARGET)),)
	CFLAGS += -fpic
endif

ifeq ($(BITS),32)
	CFLAGS += -m32
endif

ifeq ($(BITS),64)
	CFLAGS += -m64
endif

CFLAGS += -fno-exceptions -fno-rtti -felide-constructors # -static-libgcc 

OBJECTS = $(patsubst %.cpp,%.o,$(filter %.cpp, $(SRC))) $(patsubst %.c,%.o,$(filter %.c, $(SRC)))

CC := gcc
CCX := gcc

%.o: %.cpp
	$(CCX) -c $(CFLAGS) $(DEFINES) $(SMDEF) $(SMINC) $(INCLUDES) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) $(DEFINES) $(SMDEF) $(SMINC) $(INCLUDES) -o $@ $<

.PHONY: $(TARGET)

ifneq ($(findstring .so,$(TARGET)),)
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(DEFINES) $(SMDEF) $(SMINC) $(INCLUDES) -o $@ -shared -Wl,-soname,$@ $? -Wl,-Bstatic $(STATICLIBS) -Wl,-Bdynamic $(SHAREDLIBS) $(SMLIB)
endif

ifneq ($(findstring .a,$(TARGET)),)
$(TARGET): $(OBJECTS)
	$(AR) rcs $@ $^
endif

ifeq ($(findstring .,$(TARGET)),)
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(DEFINES) $(SMDEF) $(SMINC) $(INCLUDES) -o $@ $^ -static-libgcc -Wl,-Bstatic $(STATICLIBS) -Wl,-Bdynamic $(SHAREDLIBS) $(SMLIB)
endif

.PHONY: $(DEPENDS)
$(DEPENDS):
	$(MAKE) $(MFLAGS) -C $(dir $@) -f $(notdir $@) $(MAKECMDGOALS)

.PHONY: clean distclean
clean distclean: $(DEPENDS)
	-rm *.o $(TARGET)
	-rm ./$(BUILD)/*
	-rmdir ./$(BUILD)/

.PHONY: all
all: $(DEPENDS) $(TARGET)

.PHONY: copy 
copy:
	-mkdir ./$(BUILD)/
	cp $(TARGET) ./$(BUILD)/

################################# END

# If I decide to use C instead of C++ ( but no static functions support ):
#  -std=c99

# -lstdc++

# Linking libstdc++ statically
# 	http://www.trilithium.com/johan/2005/06/static-libstdc/
#  tip: use gcc instead of g++
#
# http://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html
# debugging information: -g
# generate warnings -Wall
# Using -fpic option usually generates smaller and faster code, but will have platform-dependent limitations ...

# Linking against static library: gcc -static main.c -L. -lmean -o statically_linked
# Linking against shared library: gcc main.c -o dynamically_linked -L. -lmean

#-shared-libgcc
#-static-libgcc
#	On systems that provide libgcc as a shared library, these options force the use of either the shared or static version respectively.  If no shared
#	version of libgcc was built when the compiler was configured, these options have no effect.


#Linking with a mix of shared and static libraries
# http://linux4u.jinr.ru/usoft/WWW/www_debian.org/Documentation/elf/node18.html
# gcc -o main main.o -Wl,-Bstatic -lfoo -Wl,-Bdynamic -lbar

# __gxx_personality_v0 ...error: use g++ instead of gcc
# 

#ERROR: error while loading shared libraries: libstdc++.so.6: cannot open shared object file: No such file or directory
# If you install libstdc++.so.6 in a non-standard location, you need to tell the dynamic linker where to find it.  This can be done by setting
# the environment variable LD_LIBRARY_PATH to the directory where libstdc++.so.6 can be found.  Or by using -Wl,-rpath,DIR when you
# link.  Or, on a GNU/Linux system, by adding the directory to /etc/ld.so.conf and friends and running ldconfig.
