# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2019-20

SOURCES = main.c fs.c sync.c 
SOURCES+= lib/bst.c lib/hash.c lib/inodes.c	../client/tecnicofs-client-api.c
OBJS_RWLOCK = $(SOURCES:%.c=%-rwlock.o)
OBJS = $(OBJS_RWLOCK)
CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../ -g
LDFLAGS=-lm -pthread
TARGETS = tecnicofs-rwlock

.PHONY: all clean

all: $(TARGETS)

$(TARGETS):
	$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)

### RWLOCK ###
lib/hash-rwlock.o: CFLAGS+=-DRWLOCK
lib/hash-rwlock.o: lib/hash.c lib/hash.h							

lib/bst-rwlock.o: CFLAGS+=-DRWLOCK
lib/bst-rwlock.o: lib/bst.c lib/bst.h

lib/inodes-rwlock.o: CFLAGS+=-DRWLOCK
lib/inodes-rwlock.o: lib/inodes.c lib/inodes.h

../client/tecnicofs-client-api-rwlock.o: CFLAGS+=-DRWLOCK
../client/tecnicofs-client-api-rwlock.o: ../client/tecnicofs-client-api.c ../client/tecnicofs-client-api.h ../client/tecnicofs-api-constants.h

fs-rwlock.o: CFLAGS+=-DRWLOCK
fs-rwlock.o: fs.c fs.h lib/bst.h

sync-rwlock.o: CFLAGS+=-DRWLOCK
sync-rwlock.o: sync.c sync.h constants.h

main-rwlock.o: CFLAGS+=-DRWLOCK
main-rwlock.o: main.c fs.h lib/hash.h lib/bst.h lib/inodes.h lib/inodes.c constants.h lib/timer.h sync.h ../client/tecnicofs-client-api.h ../client/tecnicofs-api-constants.h
tecnicofs-rwlock: lib/hash-rwlock.o lib/bst-rwlock.o lib/inodes-rwlock.o fs-rwlock.o sync-rwlock.o ../client/tecnicofs-client-api-rwlock.o main-rwlock.o	


%.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo Cleaning...
	rm -f $(OBJS) $(TARGETS)
