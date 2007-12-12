CC=gcc

.PHONY: all
all:
	$(CC) -c $(CFLAGS) -o sqlite3.o src/sqlite3.c
	$(AR) rcs libsqlite3.a sqlite3.o

.PHONY: clean
clean:
	-rm sqlite3.o libsqlite3.a

