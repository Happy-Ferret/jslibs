include ../subdirs.mk

TARGET_FILES=$(INT_DIR)libnedmalloc.a

CC=gcc
CFLAGS += -fPIC

ifeq ($(BUILD),opt)
	CFLAGS += -O3 -DINSECURE=1
else
	CFLAGS += -O0 -g
endif


$(TARGET_FILES):
	mkdir -p $(INT_DIR)
	$(CC) -c -o nedmalloc.o $(CFLAGS) nedmalloc.c
	$(AR) rcs libnedmalloc.a nedmalloc.o
	mv nedmalloc.o libnedmalloc.a $(INT_DIR)

all:: $(TARGET_FILES)

clean::
	-[ -d "./$(INT_DIR)" ] && rm -r ./$(INT_DIR)
