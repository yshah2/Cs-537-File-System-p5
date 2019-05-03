CC=gcc
CFLAGS= -Wall

all: xv6_fsck.c
	$(CC) xv6_fsck.c -o xv6_fsck $(CFLAGS)

clean:
	rm -f xv6_fsck
