CC := gcc
AR := ar

BUILD ?= opt

ifeq ($(BUILD),dbg)
	CFLAGS += -Wall -g3 -O0
	SMINC = -I../js/src/Linux_All_DBG.OBJ -I../js/src
	SMLIB = -Wl,-Bdynamic -L../js/src/Linux_All_DBG.OBJ -ljs
else
	CFLAGS += -Wall -O3 -s -funroll-loops
	SMINC = -I../js/src/Linux_All_OPT.OBJ -I../js/src
	SMLIB = -Wl,-Bdynamic -L../js/src/Linux_All_OPT.OBJ -ljs
endif


ifneq ($(findstring .so,$(TARGET)),)
	CFLAGS += -fpic
endif

CFLAGS += -fno-exceptions -fno-rtti -felide-constructors # -static-libgcc 

OBJECTS = $(patsubst %.cpp,%.o,$(filter %.cpp, $(SRC))) $(patsubst %.c,%.o,$(filter %.c, $(SRC)))



.PHONY: $(TARGET)

ifneq ($(findstring .so,$(TARGET)),)
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(DEFINES) $(SMINC) $(INCLUDES) -o $@ -shared -Wl,-soname,$@ $? -Wl,-Bstatic $(STATICLIBS) -Wl,-Bdynamic $(SHAREDLIBS) $(SMLIB)
endif

ifneq ($(findstring .a,$(TARGET)),)
$(TARGET): $(OBJECTS)
	$(AR) rcs $@ $^
endif

ifeq ($(findstring .,$(TARGET)),)
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(DEFINES) $(SMINC) $(INCLUDES) -o $@ $^ -static-libgcc -Wl,-Bstatic $(STATICLIBS) -Wl,-Bdynamic $(SHAREDLIBS) $(SMLIB)
	 mv $@ $(basename $@)
endif


#%.o: %.cpp
#	$(CC) -c $(CFLAGS) $(DEFINES) $(SMINC) $(INCLUDES) -o $@ $<
#
#%.o: %.c
#	$(CC) -c $(CFLAGS) $(DEFINES) $(SMINC) $(INCLUDES) -o $@ $<
#
#%.a: $(OBJECTS)
#	$(AR) rcs $@ $^
#
#%.so: $(OBJECTS)
#	$(CC) $(CFLAGS) $(DEFINES) $(SMINC) $(INCLUDES) -o $@ -shared -Wl,-soname,$@ $? -Wl,-Bstatic $(STATICLIBS) -Wl,-Bdynamic $(SHAREDLIBS) $(SMLIB)
#
#%.bin: $(OBJECTS)
#	$(CC) $(CFLAGS) $(DEFINES) $(SMINC) $(INCLUDES) -o $@ $^ -static-libgcc -Wl,-Bstatic $(STATICLIBS) -Wl,-Bdynamic $(SHAREDLIBS) $(SMLIB)
#	mv $@ $(basename $@)
#
#

.PHONY: $(DEPENDS)
$(DEPENDS):
	$(MAKE) $(MFLAGS) -C $(dir $@) -f $(notdir $@) $(MAKECMDGOALS)

.PHONY: clean
clean: $(DEPENDS)
	-rm -f *.o $(TARGET) $(basename $(TARGET))

.PHONY: all
all: $(DEPENDS) $(TARGET)

.PHONY: install
install: ;

.PHONY: copy
copy: ;

.DEFAULT_GOAL := all



################################# END


#dependenses:
#	for d in $(DEPENDS); do $(MAKE) -C $$d $(MAKECMDGOALS); done


#.PRECIOUS: %.o


# If I decide to use C instead of C++ ( but no static functions support ):
#  -std=c99

# -lstdc++


# Linking libstdc++ statically
# 	http://www.trilithium.com/johan/2005/06/static-libstdc/
#  tip: use gcc instead of g++
#
# make manual: http://www.gnu.org/software/make/manual/make.html#Automatic-Variables
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
