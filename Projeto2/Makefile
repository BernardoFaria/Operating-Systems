# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2019-20

SOURCES = main.c fs.c sync.c
SOURCES+= lib/bst.c
OBJS_NOSYNC = $(SOURCES:%.c=%.o)
OBJS_MUTEX  = $(SOURCES:%.c=%-mutex.o)
OBJS_RWLOCK = $(SOURCES:%.c=%-rwlock.o)
OBJS = $(OBJS_NOSYNC) $(OBJS_MUTEX) $(OBJS_RWLOCK)
CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../ -g
LDFLAGS=-lm -pthread
TARGETS = tecnicofs-nosync tecnicofs-mutex tecnicofs-rwlock

.PHONY: all clean

all: $(TARGETS)

$(TARGETS):
	$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)


### no sync ###
lib/bst.o: lib/bst.c lib/bst.h
fs.o: fs.c fs.h lib/bst.h
sync.o: sync.c sync.h constants.h
main.o: main.c fs.h lib/bst.h constants.h lib/timer.h sync.h
tecnicofs-nosync: lib/bst.o fs.o sync.o main.o

### MUTEX ###
lib/bst-mutex.o: CFLAGS+=-DMUTEX
lib/bst-mutex.o: lib/bst.c lib/bst.h

fs-mutex.o: CFLAGS+=-DMUTEX
fs-mutex.o: fs.c fs.h lib/bst.h

sync-mutex.o: CFLAGS+=-DMUTEX
sync-mutex.o: sync.c sync.h constants.h

main-mutex.o: CFLAGS+=-DMUTEX
main-mutex.o: main.c fs.h lib/bst.h constants.h lib/timer.h sync.h
tecnicofs-mutex: lib/bst-mutex.o fs-mutex.o sync-mutex.o main-mutex.o

### RWLOCK ###
lib/bst-rwlock.o: CFLAGS+=-DRWLOCK
lib/bst-rwlock.o: lib/bst.c lib/bst.h

fs-rwlock.o: CFLAGS+=-DRWLOCK
fs-rwlock.o: fs.c fs.h lib/bst.h

sync-rwlock.o: CFLAGS+=-DRWLOCK
sync-rwlock.o: sync.c sync.h constants.h

main-rwlock.o: CFLAGS+=-DRWLOCK
main-rwlock.o: main.c fs.h lib/bst.h constants.h lib/timer.h sync.h
tecnicofs-rwlock: lib/bst-rwlock.o fs-rwlock.o sync-rwlock.o main-rwlock.o


%.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo Cleaning...
	rm -f $(OBJS) $(TARGETS)
