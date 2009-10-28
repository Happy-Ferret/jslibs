ifeq ($(MAKECMDGOALS),)
$(error NO GOAL SPECIFIED)
endif

CC := gcc
CXX := gcc

.SUFFIXES: .a .o .c .cpp .h

BUILD ?= opt
BITS ?= 32

INT_DIR = $(shell uname)_$(BUILD)_$(BITS)/

# -Wfatal-errors
ifeq ($(BUILD),dbg)
	CFLAGS += -Wall -g3 -O0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -DDEBUG -I../../libs/js/$(INT_DIR) -I../../libs/js/src
	#CFLAGS += -DJS_GCMETER -DJS_HASHMETER -DJS_GC_ZEAL -DJS_DUMP_PROPTREE_STATS -DJS_ARENAMETER
	# -fmudflap // http://gcc.gnu.org/wiki/Mudflap_Pointer_Debugging
else
	CFLAGS += -Wall -g0 -O3 -s -funroll-loops -I../../libs/js/$(INT_DIR) -I../../libs/js/src
endif

CFLAGS += -Wno-unused-parameter
LDFLAGS += -Wl,-Bdynamic -L../../libs/js/$(INT_DIR) -lmozjs
# -static-libgcc -Wl,-Bstatic,-lstdc++
#,-lgcc_s

ifeq ($(BITS),64)
	CFLAGS += -m64 -fPIC
else
	CFLAGS += -m32
endif

CFLAGS += $(INCLUDES) $(DEFINES) -fno-exceptions -fno-rtti -felide-constructors
# -static-libgcc 

OBJECTS = $(patsubst %.cpp,$(INT_DIR)%.o,$(filter %.cpp, $(SRC))) $(patsubst %.c,$(INT_DIR)%.o,$(filter %.c, $(SRC)))

$(INT_DIR):
	mkdir -p $(INT_DIR)

$(INT_DIR)%.o: %.cpp 
	$(CXX) -c $(CFLAGS) -o $@ $<

$(INT_DIR)%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<



ifneq ($(findstring .so,$(TARGET)),)
$(INT_DIR)$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -fpic -o $@ -shared -Wl,-soname,$@ $? -Wl,-Bstatic $(STATICLIBS) -Wl,-Bdynamic $(SHAREDLIBS) $(LDFLAGS)
endif

ifneq ($(findstring .a,$(TARGET)),)
$(INT_DIR)$(TARGET): $(OBJECTS)
	$(AR) rcs $@ $^
endif

ifeq ($(findstring .,$(TARGET)),)
$(INT_DIR)$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -static-libgcc -Wl,-Bstatic $(STATICLIBS) -Wl,-Bdynamic $(SHAREDLIBS) $(LDFLAGS)
endif



.PHONY: $(DEPENDS)
$(DEPENDS):
	$(MAKE) $(MFLAGS) -C $(dir $@) -f $(notdir $@) $(MAKECMDGOALS)

.PHONY: clean distclean
clean distclean:: $(DEPENDS)
	-rm $(OBJECTS)
	-rm $(INT_DIR)$(TARGET)
	-rmdir ./$(INT_DIR)

.PHONY: all
all:: $(DEPENDS) $(INT_DIR) $(INT_DIR)$(TARGET)

.PHONY: copy 
copy::
	mkdir -p $(DEST_DIR)
	cp ./$(INT_DIR)$(TARGET) $(DEST_DIR)


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
