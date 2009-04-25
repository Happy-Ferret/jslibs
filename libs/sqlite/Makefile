include ../subdirs.mk

TARGET_FILES=$(INT_DIR)libsqlite3.a

CC=gcc
CFLAGS += -fPIC -DSQLITE_THREADSAFE=0 -DSQLITE_SYSTEM_MALLOC

ifeq ($(BUILD),opt)
	CFLAGS += -O3
else
	CFLAGS += -O0 -g
endif

$(TARGET_FILES):
	mkdir -p $(INT_DIR)
	cd src && $(CC) -c -o sqlite3.o $(CFLAGS) sqlite3.c
	cd src && $(AR) rcs libsqlite3.a sqlite3.o
	cp ./src/libsqlite3.a $(INT_DIR)
	cd src && rm sqlite3.o libsqlite3.a

all:: $(TARGET_FILES)

clean::
	-[ -d "./$(INT_DIR)" ] && rm -r ./$(INT_DIR)
