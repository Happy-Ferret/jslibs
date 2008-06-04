include ../subdirs.mk

TARGET_FILES = ./src/libsqlite3.a

CC=gcc
CFLAGS += -fPIC

ifeq ($(BUILD),opt)
CFLAGS += -O3
else
CFLAGS += -O0 -g
endif

./src/libsqlite3.a:
	cd src && $(CC) -c -o sqlite3.o $(CFLAGS) sqlite3.c
	cd src && $(AR) rcs libsqlite3.a sqlite3.o

all:: ./src/libsqlite3.a

clean::
	-cd src && rm sqlite3.o libsqlite3.a
